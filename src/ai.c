/**
 * @file ai.c
 * @brief IA Krojanty v2 : negamax / alpha-beta avec simulation exacte des règles.
 *
 * Points clés par rapport à la v1 :
 *  - make_move() reproduit game_apply_move()/game_apply_seultou()/game_apply_linca()/
 *    check_king_plus_one_rule()/game_check_victory_conditions() de game.c, y compris
 *    la mise à jour du territoire (ctrl) et le décompte des tours (turns_left).
 *  - La fin de partie est évaluée comme le jeu le fait : score = cases contrôlées + soldats.
 *  - Recherche à temps borné (approfondissement itératif), TT Zobrist, PVS, LMR,
 *    killers, history, extension "roi menacé", quiescence sur les captures.
 */

#define _POSIX_C_SOURCE 200809L
#include "ai.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NSQ     81
#define MATE    30000
#define MAXPLY  100
#define QMAX    8

/* ====================================================================== */
/* Représentation compacte                                                */
/* ====================================================================== */

typedef struct {
    signed char b[NSQ];      /* 0 vide, 1 SB, 2 KB, 3 SR, 4 KR (enum Piece) */
    signed char ctrl[NSQ];   /* 0 aucun, 1 bleu, 2 rouge                    */
    signed char cnt[5];      /* nombre de pièces par type                   */
    signed char turn;        /* 1 bleu, 2 rouge                             */
    signed char over;
    signed char winner;      /* 0 nul / personne, 1 bleu, 2 rouge           */
    short turns_left;
    unsigned long long hash;
} Pos;

typedef struct {
    unsigned char f, t;
    short gain;
    int ord;
} Mv;

typedef struct {
    unsigned long long key;
    short score;
    signed char depth;
    unsigned char flag;      /* 0 exact, 1 borne basse, 2 borne haute */
    unsigned char f, t;
} TTE;

static const int DR[4] = {-1, 1, 0, 0};
static const int DC[4] = {0, 0, -1, 1};

static inline int  owner(int pc) { return pc == 0 ? 0 : (pc <= 2 ? 1 : 2); }
static inline bool is_king(int pc) { return pc == 2 || pc == 4; }
static inline bool inb(int r, int c) { return (unsigned)r < 9u && (unsigned)c < 9u; }
static inline int  sgn(int x) { return (x > 0) - (x < 0); }
static inline bool is_city(int sq) { return sq == 0 || sq == 80; }

/* ====================================================================== */
/* Zobrist                                                                */
/* ====================================================================== */

static unsigned long long ZP[5][NSQ], ZC[3][NSQ], ZTURN, ZT[128];
static bool z_ready = false;

static unsigned long long splitmix(unsigned long long *s) {
    unsigned long long z = (*s += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

static void zinit(void) {
    if (z_ready) return;
    unsigned long long s = 0x4B524F4A414E5459ULL;
    for (int i = 0; i < 5; i++) for (int j = 0; j < NSQ; j++) ZP[i][j] = splitmix(&s);
    for (int i = 0; i < 3; i++) for (int j = 0; j < NSQ; j++) ZC[i][j] = splitmix(&s);
    ZTURN = splitmix(&s);
    for (int i = 0; i < 128; i++) ZT[i] = splitmix(&s);
    z_ready = true;
}

static unsigned long long hash_full(const Pos *p) {
    unsigned long long h = 0;
    for (int sq = 0; sq < NSQ; sq++) {
        if (p->b[sq])    h ^= ZP[(int)p->b[sq]][sq];
        if (p->ctrl[sq]) h ^= ZC[(int)p->ctrl[sq]][sq];
    }
    if (p->turn == 2) h ^= ZTURN;
    h ^= ZT[p->turns_left];
    return h;
}

static inline void put(Pos *p, int sq, int pc) {
    int old = p->b[sq];
    if (old) { p->hash ^= ZP[old][sq]; p->cnt[old]--; }
    p->b[sq] = (signed char)pc;
    if (pc)  { p->hash ^= ZP[pc][sq];  p->cnt[pc]++; }
}

static inline void setctrl(Pos *p, int sq, int c) {
    int o = p->ctrl[sq];
    if (o) p->hash ^= ZC[o][sq];
    p->ctrl[sq] = (signed char)c;
    if (c) p->hash ^= ZC[c][sq];
}

/* ====================================================================== */
/* Conversions                                                            */
/* ====================================================================== */

static void pos_finish(Pos *p) {
    memset(p->cnt, 0, sizeof p->cnt);
    for (int sq = 0; sq < NSQ; sq++)
        if (p->b[sq] >= 1 && p->b[sq] <= 4) p->cnt[(int)p->b[sq]]++;
    if (p->turns_left < 0) p->turns_left = 0;
    if (p->turns_left > 127) p->turns_left = 127;
    p->hash = hash_full(p);
}

static void pos_from_game(const GameState *g, Pos *p) {
    zinit();
    memset(p, 0, sizeof *p);
    for (int r = 0; r < BOARD_SIZE; r++)
        for (int c = 0; c < BOARD_SIZE; c++) {
            p->b[r * 9 + c]    = (signed char)g->board[r][c].piece;
            p->ctrl[r * 9 + c] = (signed char)g->ctrl[r][c];
        }
    p->turn = (g->turn == PLAYER_RED) ? 2 : 1;
    p->turns_left = (short)g->turns_left;
    p->over = g->game_over ? 1 : 0;
    pos_finish(p);
}

static void pos_from_state(const AI_GameState *s, Pos *p) {
    zinit();
    memset(p, 0, sizeof *p);
    for (int r = 0; r < BOARD_SIZE; r++)
        for (int c = 0; c < BOARD_SIZE; c++) {
            p->b[r * 9 + c]    = (signed char)s->board[r][c].piece;
            p->ctrl[r * 9 + c] = (signed char)s->ctrl[r][c];
        }
    p->turn = (s->current_player == AI_PLAYER_RED) ? 2 : 1;
    p->turns_left = (short)s->turns_remaining;
    p->over = s->game_over ? 1 : 0;
    pos_finish(p);
}

static void state_from_pos(const Pos *p, AI_GameState *s) {
    for (int r = 0; r < BOARD_SIZE; r++)
        for (int c = 0; c < BOARD_SIZE; c++) {
            int sq = r * 9 + c;
            s->board[r][c].piece = (Piece)p->b[sq];
            s->board[r][c].cell = sq == 0 ? CELL_CITY_BLUE : (sq == 80 ? CELL_CITY_RED : CELL_EMPTY);
            s->ctrl[r][c] = (Control)p->ctrl[sq];
        }
    s->current_player = p->turn == 2 ? AI_PLAYER_RED : AI_PLAYER_BLUE;
    s->blue_pieces = p->cnt[1] + p->cnt[2];
    s->red_pieces  = p->cnt[3] + p->cnt[4];
    s->blue_kings = p->cnt[2];
    s->red_kings  = p->cnt[4];
    s->turns_remaining = p->turns_left;
    s->game_over = p->over;
    s->material_balance = s->blue_pieces - s->red_pieces;
}

/* ====================================================================== */
/* Gain d'un coup (sans l'appliquer) : sert à l'ordonnancement/quiescence */
/* flags : 1 SEULTOU, 2 LINCA, 4 capture roi, 8 roi sur la cité          */
/* ====================================================================== */

static int move_gain_v(const Pos *p, int f, int t, int *flags, int *vic, int *vval, int *nv) {
    int piece = p->b[f], me = owner(piece), opp = 3 - me;
    int fr = f / 9, fc = f % 9, tr = t / 9, tc = t % 9;
    int dr = sgn(tr - fr), dc = sgn(tc - fc);
    int gain = 0, fl = 0, vsq = -1, k = 0;

    int er = tr + dr, ec = tc + dc;
    if (inb(er, ec)) {
        int e = er * 9 + ec, ep = p->b[e];
        if (ep && owner(ep) == opp) {
            int rr = er + dr, cc = ec + dc;
            bool ok;
            if (!inb(rr, cc)) ok = true;
            else { int bp = p->b[rr * 9 + cc]; ok = (bp == 0 || owner(bp) == me); }
            if (ok) {
                vsq = e; fl |= 1;
                if (vic) { vic[k] = e; vval[k] = is_king(ep) ? 2000 : 10; k++; }
                if (is_king(ep)) { gain += 2000; fl |= 4; } else gain += 10;
            }
        }
    }
    for (int d = 0; d < 4; d++) {
        int mr = tr + DR[d], mc = tc + DC[d], ar = tr + 2 * DR[d], ac = tc + 2 * DC[d];
        if (!inb(mr, mc) || !inb(ar, ac)) continue;
        int m = mr * 9 + mc, a = ar * 9 + ac;
        int mp = (m == vsq || m == f) ? 0 : p->b[m];
        int ap = (a == f) ? 0 : p->b[a];
        if (mp && owner(mp) == opp && ap && owner(ap) == me) {
            fl |= 2;
            if (vic) { vic[k] = m; vval[k] = is_king(mp) ? 2000 : 10; k++; }
            if (is_king(mp)) { gain += 2000; fl |= 4; } else gain += 10;
        }
    }
    if (is_king(piece) && ((me == 1 && t == 80) || (me == 2 && t == 0))) { gain += 2000; fl |= 8; }
    if (flags) *flags = fl;
    if (nv) *nv = k;
    return gain;
}

static inline int move_gain(const Pos *p, int f, int t, int *flags) {
    return move_gain_v(p, f, t, flags, NULL, NULL, NULL);
}

/* ====================================================================== */
/* Génération                                                             */
/* ====================================================================== */

static int gen_moves(const Pos *p, int side, Mv *mv, bool caps_only) {
    int n = 0;
    for (int f = 0; f < NSQ; f++) {
        int pc = p->b[f];
        if (!pc || owner(pc) != side) continue;
        int fr = f / 9, fc = f % 9;
        for (int d = 0; d < 4; d++) {
            int r = fr + DR[d], c = fc + DC[d];
            while (inb(r, c) && p->b[r * 9 + c] == 0) {
                int t = r * 9 + c;
                int g = move_gain(p, f, t, NULL);
                if (!caps_only || g > 0) {
                    mv[n].f = (unsigned char)f; mv[n].t = (unsigned char)t;
                    mv[n].gain = (short)g; mv[n].ord = 0;
                    n++;
                }
                r += DR[d]; c += DC[d];
            }
        }
    }
    return n;
}

/* ====================================================================== */
/* Application d'un coup : miroir exact de game.c                         */
/* ====================================================================== */

static void make_move(Pos *p, int f, int t) {
    const int piece = p->b[f];
    const int me = owner(piece), opp = 3 - me;
    const int fr = f / 9, fc = f % 9, tr = t / 9, tc = t % 9;
    const int dr = sgn(tr - fr), dc = sgn(tc - fc);
    bool pend = false;
    int pwin = 0;

    /* SEULTOU ? (game_can_seultou) */
    int victim_sq = -1;
    int er = tr + dr, ec = tc + dc;
    if (inb(er, ec)) {
        int e = er * 9 + ec, ep = p->b[e];
        if (ep && owner(ep) == opp) {
            int rr = er + dr, cc = ec + dc;
            bool ok;
            if (!inb(rr, cc)) ok = true;
            else { int bp = p->b[rr * 9 + cc]; ok = (bp == 0 || owner(bp) == me); }
            if (ok) victim_sq = e;
        }
    }

    put(p, f, 0);
    put(p, t, piece);
    if (!is_city(t)) setctrl(p, t, me);

    if (victim_sq >= 0) {
        int vp = p->b[victim_sq];
        put(p, victim_sq, 0);
        setctrl(p, victim_sq, 0);
        if (is_king(vp)) { pend = true; pwin = me; }
    }

    if (is_king(piece) && ((me == 1 && t == 80) || (me == 2 && t == 0))) { pend = true; pwin = me; }

    /* LINCA */
    for (int d = 0; d < 4; d++) {
        int mr = tr + DR[d], mc = tc + DC[d], ar = tr + 2 * DR[d], ac = tc + 2 * DC[d];
        if (!inb(mr, mc) || !inb(ar, ac)) continue;
        int m = mr * 9 + mc, a = ar * 9 + ac;
        int mp = p->b[m], ap = p->b[a];
        if (mp && owner(mp) == opp && ap && owner(ap) == me) {
            put(p, m, 0);
            setctrl(p, m, 0);
            if (is_king(mp)) { pend = true; pwin = me; }
        }
    }

    /* Règle "roi + 1 soldat" (écrase le vainqueur éventuel, comme game.c) */
    {
        int kb = p->cnt[2], sb = p->cnt[1], kr = p->cnt[4], sr = p->cnt[3];
        if (kb == 1 && sb == 1 && kr == 1 && sr == 1) { pend = true; pwin = 0; }
        else if (kb == 1 && sb == 1)                  { pend = true; pwin = 2; }
        else if (kr == 1 && sr == 1)                  { pend = true; pwin = 1; }
    }

    /* game_check_victory_conditions */
    if (pend) {
        p->over = 1; p->winner = (signed char)pwin;
    } else if (p->turns_left == 0) {
        int cb = 0, cr = 0;
        for (int sq = 0; sq < NSQ; sq++) {
            if (p->ctrl[sq] == 1) cb++; else if (p->ctrl[sq] == 2) cr++;
        }
        int sb = cb + p->cnt[1], sr = cr + p->cnt[3];
        p->over = 1;
        p->winner = sb > sr ? 1 : (sr > sb ? 2 : 0);
    } else {
        /* game_switch_turn */
        p->hash ^= ZT[p->turns_left];
        p->turns_left--;
        p->hash ^= ZT[p->turns_left];
    }
    p->hash ^= ZTURN;
    p->turn = (signed char)opp;
}

/* ====================================================================== */
/* Heuristiques                                                           */
/* ====================================================================== */

static int find_king(const Pos *p, int side) {
    int k = side == 1 ? 2 : 4;
    for (int sq = 0; sq < NSQ; sq++) if (p->b[sq] == k) return sq;
    return -1;
}

/* Le roi en ksq peut-il rejoindre la cité cible d'un seul glissement ? */
static bool line_clear(const Pos *p, int ksq, int city) {
    int kr = ksq / 9, kc = ksq % 9, cr = city / 9, cc = city % 9;
    if (p->b[city] != 0) return false;
    if (kr == cr) {
        int s = sgn(cc - kc);
        for (int c = kc + s; c != cc; c += s) if (p->b[kr * 9 + c]) return false;
        return true;
    }
    if (kc == cc) {
        int s = sgn(cr - kr);
        for (int r = kr + s; r != cr; r += s) if (p->b[r * 9 + kc]) return false;
        return true;
    }
    return false;
}

static int friend_neighbors(const Pos *p, int sq, int side) {
    int n = 0, r = sq / 9, c = sq % 9;
    for (int d = 0; d < 4; d++) {
        int nr = r + DR[d], nc = c + DC[d];
        if (inb(nr, nc) && owner(p->b[nr * 9 + nc]) == side) n++;
    }
    return n;
}

/* Approximation rapide : un coup adverse capturerait-il le roi de 'side' ? */
static bool king_in_danger(const Pos *p, int side) {
    int k = find_king(p, side);
    if (k < 0) return false;
    int kr = k / 9, kc = k % 9, opp = 3 - side;
    for (int d = 0; d < 4; d++) {
        int tr = kr + DR[d], tc = kc + DC[d];
        if (!inb(tr, tc) || p->b[tr * 9 + tc] != 0) continue;
        int br = kr - DR[d], bc = kc - DC[d];
        bool behind_in = inb(br, bc);
        int bp = behind_in ? p->b[br * 9 + bc] : 0;
        /* SEULTOU : un adverse glisse vers le roi et s'arrête en t */
        if (!behind_in || bp == 0 || owner(bp) == opp) {
            int r = tr + DR[d], c = tc + DC[d];
            while (inb(r, c) && p->b[r * 9 + c] == 0) { r += DR[d]; c += DC[d]; }
            if (inb(r, c) && owner(p->b[r * 9 + c]) == opp) return true;
        }
        /* LINCA : adverse derrière le roi, un autre adverse peut venir en t */
        if (behind_in && owner(bp) == opp) {
            for (int e = 0; e < 4; e++) {
                int r = tr + DR[e], c = tc + DC[e];
                while (inb(r, c) && p->b[r * 9 + c] == 0) { r += DR[e]; c += DC[e]; }
                if (inb(r, c) && owner(p->b[r * 9 + c]) == opp) return true;
            }
        }
    }
    return false;
}

/* ====================================================================== */
/* Évaluation                                                             */
/* ====================================================================== */

static int eval_blue(const Pos *p) {
    int cb = 0, cr = 0, kb = -1, kr = -1, mob = 0;
    for (int sq = 0; sq < NSQ; sq++) {
        int c = p->ctrl[sq];
        if (c == 1) cb++; else if (c == 2) cr++;
        int pc = p->b[sq];
        if (!pc) continue;
        int r = sq / 9, cc = sq % 9, fr = 0;
        for (int d = 0; d < 4; d++) {
            int nr = r + DR[d], nc = cc + DC[d];
            if (inb(nr, nc) && p->b[nr * 9 + nc] == 0) fr++;
        }
        if (pc == 1) mob += fr; else if (pc == 3) mob -= fr;
        else if (pc == 2) kb = sq; else kr = sq;
    }
    int tl = p->turns_left; if (tl > 64) tl = 64; if (tl < 0) tl = 0;
    int wc = 3 + (37 * (64 - tl)) / 64;                    /* le territoire pèse de plus en plus */
    int s = 70 * (p->cnt[1] - p->cnt[3]) + wc * (cb - cr) + mob;

    /* attrition : 1 roi + 1 soldat = défaite */
    if (p->cnt[3] <= 2) s += 60 * (3 - p->cnt[3]);
    if (p->cnt[1] <= 2) s -= 60 * (3 - p->cnt[1]);

    if (kb >= 0 && kr >= 0) {
        int dB = (8 - kb / 9) + (8 - kb % 9), dR = kr / 9 + kr % 9;
        s += 4 * (dR - dB);
        s += 10 * friend_neighbors(p, kb, 1) - 10 * friend_neighbors(p, kr, 2);
        if (p->turn != 1 && line_clear(p, kb, 80)) s += 250;  /* menace de victoire */
        if (p->turn != 2 && line_clear(p, kr, 0))  s -= 250;
    }
    return s;
}

static int eval(const Pos *p) {
    int s = eval_blue(p);
    return (p->turn == 1 ? s : -s) + 6;
}

/* ====================================================================== */
/* Recherche                                                              */
/* ====================================================================== */

static struct {
    TTE *tt;
    unsigned long long mask;
    long long deadline;
    bool stop;
    unsigned long long nodes, cutoffs;
    int ext_limit;
    short killer[MAXPLY + 4][2];
    int hist[3][NSQ * NSQ];
} S;

static long long now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL;
}

static inline void tick(void) {
    if ((++S.nodes & 2047ULL) == 0 && now_ms() >= S.deadline) S.stop = true;
}

static inline int terminal_score(const Pos *p, int ply) {
    if (p->winner == 0) return 0;
    return (p->winner == p->turn) ? (MATE - ply) : -(MATE - ply);
}

static inline int to_tt(int s, int ply)   { return s > MATE - 300 ? s + ply : (s < -MATE + 300 ? s - ply : s); }
static inline int from_tt(int s, int ply) { return s > MATE - 300 ? s - ply : (s < -MATE + 300 ? s + ply : s); }

static void sort_moves_desc(Mv *mv, int n, int i) {
    int bi = i;
    for (int j = i + 1; j < n; j++) if (mv[j].ord > mv[bi].ord) bi = j;
    if (bi != i) { Mv t = mv[i]; mv[i] = mv[bi]; mv[bi] = t; }
}

#ifndef THREAT
#define THREAT 1
#endif

/* Pénalité statique : menaces adverses multiples que le trait ne peut pas toutes parer. */
static int threat_penalty(const Pos *p) {
    const int opp = 3 - p->turn;
    short val[NSQ];
    memset(val, 0, sizeof val);
    for (int f = 0; f < NSQ; f++) {
        int pc = p->b[f];
        if (!pc || owner(pc) != opp) continue;
        int fr = f / 9, fc = f % 9;
        for (int d = 0; d < 4; d++) {
            int r = fr + DR[d], c = fc + DC[d];
            while (inb(r, c) && p->b[r * 9 + c] == 0) {
                int vic[5], vv[5], nv = 0;
                move_gain_v(p, f, r * 9 + c, NULL, vic, vv, &nv);
                for (int i = 0; i < nv; i++) if (vv[i] > val[vic[i]]) val[vic[i]] = (short)vv[i];
                r += DR[d]; c += DC[d];
            }
        }
    }
    int ns = 0, kv = 0;
    for (int sq = 0; sq < NSQ; sq++) {
        if (val[sq] >= 2000) kv = 1; else if (val[sq] > 0) ns++;
    }
    int unsaved = kv ? ns : (ns > 0 ? ns - 1 : 0);
    return 75 * unsaved + (kv ? 100 : 0);
}

static int qsearch(Pos *p, int alpha, int beta, int ply, int qd) {
    tick();
    if (S.stop) return 0;
    if (p->over) return terminal_score(p, ply);
    int stand = eval(p);
    if (qd >= QMAX || ply >= MAXPLY - 1) return stand;
    if (stand >= beta) return stand;
#if THREAT
    stand -= threat_penalty(p);
    if (stand >= beta) return stand;
#endif
    if (stand > alpha) alpha = stand;

    Mv mv[AI_MAX_MOVES];
    int n = gen_moves(p, p->turn, mv, true);
    for (int i = 0; i < n; i++) mv[i].ord = mv[i].gain;
    int best = stand;
    for (int i = 0; i < n; i++) {
        sort_moves_desc(mv, n, i);
        Pos c = *p;
        make_move(&c, mv[i].f, mv[i].t);
        int sc = -qsearch(&c, -beta, -alpha, ply + 1, qd + 1);
        if (S.stop) return 0;
        if (sc > best) best = sc;
        if (sc > alpha) alpha = sc;
        if (alpha >= beta) { S.cutoffs++; break; }
    }
    return best;
}

static int search(Pos *p, int depth, int alpha, int beta, int ply) {
    tick();
    if (S.stop) return 0;
    if (p->over) return terminal_score(p, ply);
    if (ply >= MAXPLY - 1) return eval(p);

    /* élagage par distance de mat */
    if (alpha < -(MATE - ply)) alpha = -(MATE - ply);
    if (beta > MATE - ply - 1) beta = MATE - ply - 1;
    if (alpha >= beta) return alpha;

    const int me = p->turn;
    if (depth <= 0) {
        if (ply < S.ext_limit && king_in_danger(p, me)) depth = 1;   /* extension : roi menacé */
        else return qsearch(p, alpha, beta, ply, 0);
    }

    const int alpha0 = alpha;
    int ttf = -1, ttt = -1;
    TTE *e = &S.tt[p->hash & S.mask];
    if (e->key == p->hash) {
        ttf = e->f; ttt = e->t;
        if (e->depth >= depth) {
            int sc = from_tt(e->score, ply);
            if (e->flag == 0) return sc;
            if (e->flag == 1 && sc > alpha) alpha = sc;
            else if (e->flag == 2 && sc < beta) beta = sc;
            if (alpha >= beta) return sc;
        }
    }

    Mv mv[AI_MAX_MOVES];
    int n = gen_moves(p, me, mv, false);
    if (n == 0) return -(MATE - ply);

    for (int i = 0; i < n; i++) {
        Mv *m = &mv[i];
        int k = m->f * NSQ + m->t;
        if (m->f == ttf && m->t == ttt)      m->ord = 100000000;
        else if (m->gain > 0)                m->ord = 1000000 + m->gain * 100;
        else if (k == S.killer[ply][0])      m->ord = 900000;
        else if (k == S.killer[ply][1])      m->ord = 800000;
        else                                 m->ord = S.hist[me][k] > 700000 ? 700000 : S.hist[me][k];
    }

    int best = -MATE - 1, bf = mv[0].f, bt = mv[0].t;
    for (int i = 0; i < n; i++) {
        sort_moves_desc(mv, n, i);
        Pos c = *p;
        make_move(&c, mv[i].f, mv[i].t);
        int sc;
        if (i == 0) {
            sc = -search(&c, depth - 1, -beta, -alpha, ply + 1);
        } else {
            int red = (depth >= 3 && i >= 6 && mv[i].gain == 0 && mv[i].ord < 800000) ? 1 : 0;
            sc = -search(&c, depth - 1 - red, -alpha - 1, -alpha, ply + 1);
            if (sc > alpha && red) sc = -search(&c, depth - 1, -alpha - 1, -alpha, ply + 1);
            if (sc > alpha && sc < beta) sc = -search(&c, depth - 1, -beta, -alpha, ply + 1);
        }
        if (S.stop) return 0;
        if (sc > best) { best = sc; bf = mv[i].f; bt = mv[i].t; }
        if (sc > alpha) alpha = sc;
        if (alpha >= beta) {
            S.cutoffs++;
            if (mv[i].gain == 0) {
                int k = mv[i].f * NSQ + mv[i].t;
                if (S.killer[ply][0] != k) { S.killer[ply][1] = S.killer[ply][0]; S.killer[ply][0] = (short)k; }
                S.hist[me][k] += depth * depth;
            }
            break;
        }
    }

    if (e->key != p->hash || depth >= e->depth) {
        e->key = p->hash;
        e->score = (short)to_tt(best, ply);
        e->depth = (signed char)depth;
        e->flag = best >= beta ? 1 : (best > alpha0 ? 0 : 2);
        e->f = (unsigned char)bf; e->t = (unsigned char)bt;
    }
    return best;
}

/* ====================================================================== */
/* Racine                                                                 */
/* ====================================================================== */

static AI_Context g_ctx;
static bool g_ctx_ready = false;
static int  g_time_override = 0;

void ai_set_time_limit(int ms) {
    g_time_override = ms > 0 ? ms : 0;
    if (g_ctx_ready) g_ctx.time_limit_ms = g_time_override ? g_time_override : AI_TIME_LIMIT_MS;
}

static void fill_move(const Pos *p, int f, int t, int score, AI_Move *out) {
    int fl = 0;
    move_gain(p, f, t, &fl);
    out->r0 = f / 9; out->c0 = f % 9; out->r1 = t / 9; out->c1 = t % 9;
    out->score = score;
    out->is_seultou = fl & 1; out->is_linca = (fl & 2) != 0;
    out->captures_king = (fl & 4) != 0; out->reaches_city = (fl & 8) != 0;
}

static bool choose_from_pos(AI_Context *ctx, const Pos *root, AI_Move *out) {
    if (root->over) return false;
    Mv rm[AI_MAX_MOVES];
    int n = gen_moves(root, root->turn, rm, false);
    if (n == 0) return false;

    for (int i = 0; i < n; i++) rm[i].ord = rm[i].gain;
    for (int i = 0; i < n; i++) sort_moves_desc(rm, n, i);

    if (n == 1) { fill_move(root, rm[0].f, rm[0].t, 0, out); return true; }

    S.tt = (TTE *)ctx->tt;
    S.mask = AI_TRANSPOSITION_SIZE - 1;
    S.stop = false; S.nodes = 0; S.cutoffs = 0;
    memset(S.killer, 0, sizeof S.killer);
    for (int s = 0; s < 3; s++) for (int i = 0; i < NSQ * NSQ; i++) S.hist[s][i] >>= 2;   /* vieillissement */

    long long t0 = now_ms();
    S.deadline = t0 + ctx->time_limit_ms;

    int maxd = ctx->search_depth;
    if (maxd > root->turns_left + 1) maxd = root->turns_left + 1;   /* au-delà : rien à chercher */
    if (maxd > MAXPLY - 8) maxd = MAXPLY - 8;

    int best_i = 0, best_score = 0, done_depth = 0;
    int cur_score[AI_MAX_MOVES];

    for (int depth = 1; depth <= maxd; depth++) {
        S.ext_limit = depth + 4;
        int alpha = -MATE - 10, beta = MATE + 10, bi = -1, bs = -MATE - 10;
        for (int i = 0; i < n; i++) {
            Pos c = *root;
            make_move(&c, rm[i].f, rm[i].t);
            int sc;
            if (i == 0) {
                sc = -search(&c, depth - 1, -beta, -alpha, 1);
            } else {
                sc = -search(&c, depth - 1, -alpha - 1, -alpha, 1);
                if (sc > alpha && sc < beta && !S.stop) sc = -search(&c, depth - 1, -beta, -alpha, 1);
            }
            if (S.stop) break;
            cur_score[i] = sc;
            if (sc > bs) { bs = sc; bi = i; }
            if (sc > alpha) alpha = sc;
        }
        if (S.stop) {
            if (bi >= 0) { best_i = bi; best_score = bs; }   /* coup entièrement évalué dans cette itération */
            break;
        }
        best_i = bi; best_score = bs; done_depth = depth;

        /* le meilleur coup passe en tête, les autres triés par score */
        Mv bm = rm[bi];
        for (int i = bi; i > 0; i--) { rm[i] = rm[i - 1]; cur_score[i] = cur_score[i - 1]; }
        rm[0] = bm; cur_score[0] = bs; best_i = 0;
        for (int i = 1; i < n; i++)
            for (int j = i; j > 1 && cur_score[j] > cur_score[j - 1]; j--) {
                int ts = cur_score[j]; cur_score[j] = cur_score[j - 1]; cur_score[j - 1] = ts;
                Mv tm = rm[j]; rm[j] = rm[j - 1]; rm[j - 1] = tm;
            }

        if (bs >= MATE - 200) break;                 /* victoire forcée trouvée */
        if (now_ms() - t0 > ctx->time_limit_ms / 2) break;   /* l'itération suivante ne finirait pas */
    }

    ctx->nodes_searched = (int)S.nodes;
    ctx->alpha_beta_cutoffs = (int)S.cutoffs;
    ctx->last_depth = done_depth;
    ctx->last_score = best_score;
    printf("IA: profondeur %d, %llu noeuds, %lld ms, score %d\n",
           done_depth, S.nodes, now_ms() - t0, best_score);

    fill_move(root, rm[best_i].f, rm[best_i].t, best_score, out);
    return true;
}

/* ====================================================================== */
/* API publique                                                           */
/* ====================================================================== */

void ai_init_context(AI_Context *ctx, AI_Player player) {
    if (!ctx) return;
    zinit();
    void *old = ctx->tt;
    memset(ctx, 0, sizeof *ctx);
    ctx->tt = old ? old : calloc(AI_TRANSPOSITION_SIZE, sizeof(TTE));
    if (old) memset(old, 0, (size_t)AI_TRANSPOSITION_SIZE * sizeof(TTE));
    ctx->ai_player = player;
    ctx->search_depth = AI_MAX_DEPTH;
    ctx->time_limit_ms = AI_TIME_LIMIT_MS;
    ctx->use_opening_book = false;
}

void ai_free_context(AI_Context *ctx) {
    if (!ctx) return;
    free(ctx->tt);
    ctx->tt = NULL;
}

static bool ensure_ctx(AI_Player player) {
    if (!g_ctx_ready) {
        ai_init_context(&g_ctx, player);
        g_ctx_ready = g_ctx.tt != NULL;
    }
    g_ctx.ai_player = player;
    if (g_time_override) g_ctx.time_limit_ms = g_time_override;
    return g_ctx_ready;
}

bool ai_choose_best_move(const GameState *game, AI_Move *best_move) {
    if (!game || !best_move) return false;
    Pos root;
    pos_from_game(game, &root);
    if (!ensure_ctx(root.turn == 2 ? AI_PLAYER_RED : AI_PLAYER_BLUE)) return false;
    bool ok = choose_from_pos(&g_ctx, &root, best_move);
    if (ok) {
        char from[8], to[8];
        game_coord_to_str(best_move->r0, best_move->c0, from, sizeof from);
        game_coord_to_str(best_move->r1, best_move->c1, to, sizeof to);
        printf("IA choisit: %s -> %s%s%s%s%s\n", from, to,
               best_move->is_seultou ? " (SEULTOU)" : "",
               best_move->is_linca ? " (LINCA)" : "",
               best_move->captures_king ? " (CAPTURE ROI)" : "",
               best_move->reaches_city ? " (VICTOIRE)" : "");
    }
    return ok;
}

bool ai_choose_best_move_from_board(const Square board[BOARD_SIZE][BOARD_SIZE],
                                    AI_Player player, AI_Move *best_move) {
    if (!board || !best_move) return false;
    AI_GameState st;
    memset(&st, 0, sizeof st);
    for (int r = 0; r < BOARD_SIZE; r++)
        for (int c = 0; c < BOARD_SIZE; c++) {
            st.board[r][c] = board[r][c];
            Piece pc = board[r][c].piece;
            if (pc != PIECE_NONE && !(r == 0 && c == 0) && !(r == 8 && c == 8))
                st.ctrl[r][c] = (Control)owner((int)pc);
        }
    st.current_player = player;
    st.turns_remaining = TURN_LIMIT_TOTAL;
    Pos root;
    pos_from_state(&st, &root);
    if (!ensure_ctx(player)) return false;
    return choose_from_pos(&g_ctx, &root, best_move);
}

void appstate_to_ai_board(const Square game_board[BOARD_SIZE][BOARD_SIZE],
                          Square ai_board[BOARD_SIZE][BOARD_SIZE]) {
    if (!game_board || !ai_board) return;
    memcpy(ai_board, game_board, sizeof(Square) * BOARD_SIZE * BOARD_SIZE);
}

void ai_convert_game_state(const GameState *g, AI_GameState *s) {
    if (!g || !s) return;
    Pos p;
    pos_from_game(g, &p);
    state_from_pos(&p, s);
    s->position_score = 0;
}

int ai_evaluate_position(AI_Context *ctx, const AI_GameState *state, AI_Player player) {
    if (!state) return 0;
    Pos p;
    pos_from_state(state, &p);
    int s = eval_blue(&p);
    if (ctx) ctx->endgame_mode = ai_is_endgame(state);
    return player == AI_PLAYER_BLUE ? s : -s;
}

void ai_generate_moves(const AI_GameState *state, AI_Player player,
                       AI_Move moves[AI_MAX_MOVES], int *move_count) {
    if (!state || !moves || !move_count) return;
    Pos p;
    pos_from_state(state, &p);
    Mv mv[AI_MAX_MOVES];
    int n = gen_moves(&p, player == AI_PLAYER_RED ? 2 : 1, mv, false);
    for (int i = 0; i < n; i++) fill_move(&p, mv[i].f, mv[i].t, mv[i].gain, &moves[i]);
    *move_count = n;
}

bool ai_apply_move(AI_GameState *state, const AI_Move *m) {
    if (!state || !m) return false;
    if (!game_in_bounds(m->r0, m->c0) || !game_in_bounds(m->r1, m->c1)) return false;
    Pos p;
    pos_from_state(state, &p);
    int f = m->r0 * 9 + m->c0, t = m->r1 * 9 + m->c1;
    if (!p.b[f] || p.b[t]) return false;
    if (owner(p.b[f]) != p.turn) return false;
    if (m->r0 != m->r1 && m->c0 != m->c1) return false;
    int dr = sgn(m->r1 - m->r0), dc = sgn(m->c1 - m->c0);
    for (int r = m->r0 + dr, c = m->c0 + dc; r != m->r1 || c != m->c1; r += dr, c += dc)
        if (p.b[r * 9 + c]) return false;
    make_move(&p, f, t);
    state_from_pos(&p, state);
    return true;
}

bool ai_is_tactical_move(const AI_GameState *state, const AI_Move *m) {
    if (!state || !m) return false;
    Pos p;
    pos_from_state(state, &p);
    return move_gain(&p, m->r0 * 9 + m->c0, m->r1 * 9 + m->c1, NULL) > 0;
}

bool ai_is_endgame(const AI_GameState *state) {
    if (!state) return false;
    return (state->blue_pieces + state->red_pieces) <= 6 || state->turns_remaining <= 10;
}

unsigned long long ai_zobrist_hash(const AI_Context *ctx, const AI_GameState *state) {
    (void)ctx;
    if (!state) return 0;
    Pos p;
    pos_from_state(state, &p);
    return p.hash;
}