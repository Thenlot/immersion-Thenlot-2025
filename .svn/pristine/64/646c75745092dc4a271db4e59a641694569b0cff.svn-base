/** 
*@file game.c
*@brief Implémentation principale de la logique du jeu Krojanty
*@author sofiane.bouguerri@uha.fr
*@note Fichier documenté par hadil.abahmane@uha.fr
*/

#include "game.h"
#include "ui.h"
#include "client.h"
#include "server.h"
#include "ai.h"
#include "main.h"

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

void force_quit_gtk(void);

// ============================================================================
// Helpers logiques internes
// ============================================================================

#ifndef SEULTOU_ISOLATION
#define SEULTOU_ISOLATION 0
#endif

static const int DIRS[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};


/**
*\fn static inline Control owner_to_ctrl(Player p)
*\brief Convertit un joueur en valeur de contrôle
*\param p le joueur à convertir
*\return La valeur de contrôle correspondante
*/


// Convertit un propriétaire de pièce en indicateur de contrôle de territoire.
static inline Control owner_to_ctrl(Player p) {
    if (p == PLAYER_BLUE) return CTRL_BLUE;
    if (p == PLAYER_RED)  return CTRL_RED;
    return CTRL_NONE;
}


/**
*\fn static inline bool is_opponent_city(const GameState *game, PLayer mover, int r, int c)
*\brief Vérifie si une case est la cité adverse
*\param game Etat du jeu
*\param mover Joueur en cours
*\param r ligne de la case
*\param c colonne de la classe
*\return true si c'est la cité adverse, false sinon
 */


// Indique si la case ciblée est une cité ennemie pour le joueur courant.

static inline bool is_opponent_city(const GameState *game, Player mover, int r, int c) {
    if (!game_in_bounds(r, c)) return false;
    if (mover == PLAYER_BLUE) return game->board[r][c].cell == CELL_CITY_RED;
    if (mover == PLAYER_RED)  return game->board[r][c].cell == CELL_CITY_BLUE;
    return false;
}


/**
*\fn static bool is_orthogonal_clear_path(const GameState *game, int r0, int c0, int r1, int c1) 
*\brief Vérifie si le chemin orthogonal entre deux cases est libre
*\param game Etat du jeu
*\param r0 ligne de départ
*\param c0 colonne de départ
*\param r1 Ligne d'arrivée
*\param c1 colonne d'arrivée
*\return true si le chemin est libre, false sinon
*\return
*/


// Vérifie qu'un déplacement orthogonal ne rencontre aucun obstacle.
static bool is_orthogonal_clear_path(const GameState *game, int r0, int c0, int r1, int c1) {
    if (r0 != r1 && c0 != c1) return false;
    if (!game_in_bounds(r0, c0) || !game_in_bounds(r1, c1)) return false;
    if (game->board[r1][c1].piece != PIECE_NONE) return false;

    if (r0 == r1) {
        int step = (c1 > c0) ? 1 : -1;
        for (int c = c0 + step; c != c1; c += step)
            if (game->board[r0][c].piece != PIECE_NONE)
                return false;
    } else {
        int step = (r1 > r0) ? 1 : -1;
        for (int r = r0 + step; r != r1; r += step)
            if (game->board[r][c0].piece != PIECE_NONE)
                return false;
    }
    return true;
}


/**
*\fn static bool landing_has_friend(const GameState *game, int r0, int c0, int r1, int c1) 
*\brief Vérifie si la case d'arrivée a un ami adjacent (pour seultou)
*\param game Etat du jeu
*\param r0 Ligne de départ
*\param c0 Colonne de départ
*\param r1 Ligne d'arrivée 
*\param c1 Colonne d'arrivée
*\return true si un ami est adjacent, false sinon
*/



// Teste si la case d'arrivée conserve un allié adjacent pour l'effet SEULTOU.

static bool landing_has_friend(const GameState *game, int r0, int c0, int r1, int c1) {
#if SEULTOU_ISOLATION
    Player mover = game_piece_owner(game->board[r0][c0].piece);
    for (int d = 0; d < 4; ++d) {
        int nr = r1 + DIRS[d][0];
        int nc = c1 + DIRS[d][1];
        if (!game_in_bounds(nr, nc)) continue;
        if (nr == r0 && nc == c0) continue; // square will be empty after the move
        if (game_piece_owner(game->board[nr][nc].piece) == mover) return true;
    }
    return false;
#else
    (void)game; (void)r0; (void)c0; (void)r1; (void)c1;
    return true;
#endif
}


/**
*\fn static bool seultou_allows_landing(const GameState *game, int r0, int c0, int r1, int c1) 
*\brief Vérifie si l'atterrissage Seultou est autorisé
*\param game Etat du jeu
*\param r0 Ligne de départ
*\param c0 Colonne de départ
*\param r1 Ligne d'arrivée 
*\param c1 Colonne d'arrivée
*\return true si l'atterrissage est autorisé, false sinon
*/

// Valide les contraintes supplémentaires liées à la règle SEULTOU.
static bool seultou_allows_landing(const GameState *game, int r0, int c0, int r1, int c1) {
    return landing_has_friend(game, r0, c0, r1, c1);
}


/**
*\fn static bool check_king_plus_one_rule(GameState *game) 
*\brief Vérifie la règle du roi + un soldat
*\param game Etat du jeu
*\return true si la règle s'applique, false sinon
*/

// Applique la règle "roi + soldat" et prépare une victoire éventuelle.

static bool check_king_plus_one_rule(GameState *game) {
    int kb = 0, sb = 0, kr = 0, sr = 0;
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            switch (game->board[r][c].piece) {
                case PIECE_KING_BLUE:    kb++; break;
                case PIECE_SOLDIER_BLUE: sb++; break;
                case PIECE_KING_RED:     kr++; break;
                case PIECE_SOLDIER_RED:  sr++; break;
                default: break;
            }
        }
    }

    if (kb == 1 && sb == 1 && kr == 1 && sr == 1) {
        game->pending_victory = true;
        game->pending_winner = PLAYER_NONE;
        snprintf(game->pending_reason, sizeof(game->pending_reason),
                 "Reste 1 roi + 1 soldat de chaque côté");
        return true;
    }

    if (kb == 1 && sb == 1) {
        game->pending_victory = true;
        game->pending_winner = PLAYER_RED;
        snprintf(game->pending_reason, sizeof(game->pending_reason),
                 "Seul roi + 1 soldat côté Bleu");
        return true;
    }

    if (kr == 1 && sr == 1) {
        game->pending_victory = true;
        game->pending_winner = PLAYER_BLUE;
        snprintf(game->pending_reason, sizeof(game->pending_reason),
                 "Seul roi + 1 soldat côté Rouge");
        return true;
    }
    return false;
}

// ============================================================================
// Fonctions d'API déclarées dans game.h
// ============================================================================


/**
*\fn bool game_in_bounds(int r, int c) 
*\brief Vérifie si des coordonnées sont dans les limites du plateau
*\param r ligne
*\param c colonne
*\return true si dans les limites, false sinon
*/


// Vérifie qu'une coordonnée appartient au plateau de jeu.

bool game_in_bounds(int r, int c) {
    return r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE;
}


/**
*\fn Player game_piece_owner(Piece p) 
*\brief Retourne le propriétaire d'une pièce
*\param p la pièce 
*\return Le joueur propriétaire
*/

// Retourne le joueur possédant une pièce donnée.

Player game_piece_owner(Piece p) {
    switch (p) {
        case PIECE_SOLDIER_BLUE:
        case PIECE_KING_BLUE: return PLAYER_BLUE;
        case PIECE_SOLDIER_RED:
        case PIECE_KING_RED:  return PLAYER_RED;
        default:              return PLAYER_NONE;
    }
}



/**
*\fn const char *game_player_name(Player p)
*\brief Retourne le nom d'un joueur
*\param p le joueur
*\return Le nom du joueur
*/


// Fournit une chaîne lisible pour l'affichage du joueur.

const char *game_player_name(Player p) {
    switch (p) {
        case PLAYER_BLUE: return "Joueur Bleu";
        case PLAYER_RED:  return "Joueur Rouge";
        default:          return "Aucun";
    }
}



/**
*\fn void game_coord_to_str(int r, int c, char *out, size_t out_sz) 
*\brief Convertit des coordonnées en chainede caractères
*\param r Ligne
*\param c Colonne
*\param out Buffer de sortie
*\param out_sz Taille du buffer
*/


// Convertit une position (ligne/colonne) en notation alphabétique.
void game_coord_to_str(int r, int c, char *out, size_t out_sz) {
    int v = BOARD_SIZE - r;
    // On écrit la lettre
    if (out_sz < 2) { if (out_sz > 0) out[0] = '\0'; return; }
    out[0] = 'A' + c;
    // On écrit le chiffre (1 ou 2 chiffres)
    if (v < 10) {
        if (out_sz < 3) { out[1] = '\0'; return; }
        out[1] = '0' + v;
        out[2] = '\0';
    } else {
        if (out_sz < 4) { out[1] = '\0'; return; }
        out[1] = '0' + (v / 10);
        out[2] = '0' + (v % 10);
        out[3] = '\0';
    }
}


/**
*\fn bool game_str_to_coord(const char *str, int *r, int *c) 
*\brief Convertit une chaîne en coordonnées
*\param str La chaine à convertir
*\param r Pointeur pour stocker la ligne
*\param c Pointeur pour stocker la colonne
*\return true si la conversion réussit, false sinon
*/


// Parse une entrée utilisateur en coordonnées de plateau.

bool game_str_to_coord(const char *str, int *r, int *c) {
    if (!str || strlen(str) < 2) return false;
    char col = str[0];
    int row = atoi(&str[1]);
    if (col < 'A' || col > 'I' || row < 1 || row > BOARD_SIZE) return false;
    if (c) *c = col - 'A';
    if (r) *r = BOARD_SIZE - row;
    return true;
}


/**
*\fn void game_init(GameState *game, GameMode mode) 
*\brief Initialise l'état du jeu
*\param game Etat du jeu à initialiser
*\return mode Mode de jeu
*/

// Initialise toutes les structures du plateau et place les pièces de départ.

void game_init(GameState *game, GameMode mode) {
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            game->board[r][c].cell = CELL_EMPTY;
            game->board[r][c].piece = PIECE_NONE;
            game->ctrl[r][c] = CTRL_NONE;
        }
    }

    game->board[0][0].cell = CELL_CITY_BLUE;
    game->board[BOARD_SIZE - 1][BOARD_SIZE - 1].cell = CELL_CITY_RED;

    game->board[1][1].piece = PIECE_KING_BLUE;
    game->board[7][7].piece = PIECE_KING_RED;

    const int blue_soldiers[9][2] = {
        {0, 2}, {0, 3}, {1, 2}, {1, 3}, {2, 0}, {2, 1}, {2, 2}, {3, 0}, {3, 1}
    };
    const int red_soldiers[9][2] = {
        {5, 8}, {5, 7}, {6, 8}, {6, 7}, {6, 6}, {7, 6}, {7, 5}, {8, 6}, {8, 5}
    };

    for (int i = 0; i < 9; ++i)
        game->board[blue_soldiers[i][0]][blue_soldiers[i][1]].piece = PIECE_SOLDIER_BLUE;
    for (int i = 0; i < 9; ++i)
        game->board[red_soldiers[i][0]][red_soldiers[i][1]].piece = PIECE_SOLDIER_RED;

    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            Piece p = game->board[r][c].piece;
            if (p != PIECE_NONE &&
                game->board[r][c].cell != CELL_CITY_BLUE &&
                game->board[r][c].cell != CELL_CITY_RED) {
                game->ctrl[r][c] = owner_to_ctrl(game_piece_owner(p));
            }
        }
    }

    game->turn = PLAYER_BLUE;
    game->turns_left = TURN_LIMIT_TOTAL;
    game->score_blue = 0;
    game->score_red = 0;
    game->game_over = false;
    game->have_selection = false;
    game->sel_r = game->sel_c = -1;
    game->game_mode = mode;
    game->pending_victory = false;
    game->pending_winner = PLAYER_NONE;
    game->pending_reason[0] = '\0';

    game_update_scores(game);
}

/**
*\fn void game_reset(GameState *game) 
*\brief Réinitialise le jeu
*\param game Etat du jeu à réinitialiser
*/

// Réinitialise une partie en conservant le mode courant.

void game_reset(GameState *game) {
    if (!game) return;
    game_init(game, game->game_mode);
}



/**
*\fn bool game_can_move(const GameState *game, int r0, int c0, int r1, int c1) 
*\brief Vérifie si un mouvement est possible
*\param game Etat du jeu
*\param r0 Ligne de départ
*\param c0 Colonne de départ
*\param r1 Ligne d'arrivée 
*\param c1 Colonne d'arrivée
*\return true si le mouvement est possible, false sinon
*/


// Détermine si un déplacement simple ou SEULTOU est autorisé.

bool game_can_move(const GameState *game, int r0, int c0, int r1, int c1) {
    if (!game) return false;
    if (!game_in_bounds(r0, c0) || !game_in_bounds(r1, c1)) return false;
    if (r0 == r1 && c0 == c1) return false;

    Piece mover = game->board[r0][c0].piece;
    if (game_piece_owner(mover) != game->turn) return false;

    if (!is_orthogonal_clear_path(game, r0, c0, r1, c1)) return false;

    int dr = (r1 > r0) ? 1 : (r1 < r0 ? -1 : 0);
    int dc = (c1 > c0) ? 1 : (c1 < c0 ? -1 : 0);
    if (dr != 0 || dc != 0) {
        int er = r1 + dr;
        int ec = c1 + dc;
        if (game_in_bounds(er, ec)) {
            Player target_owner = game_piece_owner(game->board[er][ec].piece);
            if (target_owner != PLAYER_NONE && target_owner != game_piece_owner(mover)) {
                if (game_can_seultou(game, r0, c0, r1, c1, er, ec))
                    return true;
            }
        }
    }

    if (!seultou_allows_landing(game, r0, c0, r1, c1)) return false;
    return true;
}


/**
*\fn int game_apply_linca(GameState *game, int r1, int c1) 
*\brief Applique les captures LINCA après un mouvement
*\param game Etat du jeu
*\param r1 Ligne d'arrivée
*\param c1 Colonne d'arrivée
*\return Nombre de pièces capturées
*/

// Applique les captures en chaîne LINCA autour de la destination.

int game_apply_linca(GameState *game, int r1, int c1) {
    Piece moved = game->board[r1][c1].piece;
    Player me = game_piece_owner(moved);
    Player opp = (me == PLAYER_BLUE) ? PLAYER_RED : PLAYER_BLUE;
    int captured = 0;

    for (int d = 0; d < 4; ++d) {
        int dr = DIRS[d][0];
        int dc = DIRS[d][1];
        int rm = r1 + dr;
        int cm = c1 + dc;
        int ra = r1 + 2 * dr;
        int ca = c1 + 2 * dc;

        if (!game_in_bounds(rm, cm) || !game_in_bounds(ra, ca)) continue;

        Piece mid = game->board[rm][cm].piece;
        Piece after = game->board[ra][ca].piece;

        if (game_piece_owner(mid) == opp && game_piece_owner(after) == me) {
            game->board[rm][cm].piece = PIECE_NONE;
            game->ctrl[rm][cm] = CTRL_NONE;
            captured++;

            char pos[8];
            game_coord_to_str(rm, cm, pos, sizeof pos);
            printf("Capture LINCA : ennemi retiré en %s.\n", pos);

            if (mid == PIECE_KING_BLUE || mid == PIECE_KING_RED) {
                game->pending_victory = true;
                game->pending_winner = me;
                snprintf(game->pending_reason, sizeof(game->pending_reason),
                         "Exécution du roi - LINCA");
            }
        }
    }

    return captured;
}

/**
*\fn bool game_apply_move(GameState *game, int r0, int c0, int r1, int c1) 
*\brief Applique un mouvement sur le plateau
*\param game Etat du jeu
*\param r0 Ligne de départ
*\param c0 Colonne de départ
*\param r1 Ligne d'arrivée 
*\param c1 Colonne d'arrivée
*\return true si le mouvement est appliqué, false sinon
*/

// Exécute un déplacement et gère les conditions de victoire associées.

bool game_apply_move(GameState *game, int r0, int c0, int r1, int c1) {
    if (!game_can_move(game, r0, c0, r1, c1)) return false;

    Piece mover_piece = game->board[r0][c0].piece;
    Player mover = game_piece_owner(mover_piece);

    int dr = (r1 > r0) ? 1 : (r1 < r0 ? -1 : 0);
    int dc = (c1 > c0) ? 1 : (c1 < c0 ? -1 : 0);
    if (dr != 0 || dc != 0) {
        int er = r1 + dr;
        int ec = c1 + dc;
        if (game_in_bounds(er, ec)) {
            Player target_owner = game_piece_owner(game->board[er][ec].piece);
            if (target_owner != PLAYER_NONE && target_owner != mover) {
                if (game_apply_seultou(game, r0, c0, r1, c1, er, ec))
                    return true;
            }
        }
    }

    game->board[r0][c0].piece = PIECE_NONE;
    game->board[r1][c1].piece = mover_piece;

    if (game->board[r1][c1].cell != CELL_CITY_BLUE &&
        game->board[r1][c1].cell != CELL_CITY_RED) {
        game->ctrl[r1][c1] = owner_to_ctrl(mover);
    }

    char from[8], to[8];
    game_coord_to_str(r0, c0, from, sizeof from);
    game_coord_to_str(r1, c1, to, sizeof to);
    const char *who = (mover == PLAYER_BLUE) ? "Joueur Bleu" : "Joueur Rouge";
    printf("%s : %s -> %s\n", who, from, to);

    if ((mover_piece == PIECE_KING_BLUE || mover_piece == PIECE_KING_RED) &&
        is_opponent_city(game, mover, r1, c1)) {
        game->pending_victory = true;
        game->pending_winner = mover;
        snprintf(game->pending_reason, sizeof(game->pending_reason),
                 "Roi sur le camp adverse");
    }

    int ncap = game_apply_linca(game, r1, c1);
    if (ncap > 0)
        printf("=> %d capture(s) LINCA effectuée(s).\n", ncap);

    if (!game->game_over)
        check_king_plus_one_rule(game);

    return true;
}


/**
*\fn bool game_can_seultou(const GameState *game, int r0, int c0, int r, int c, int er, int ec) 
*\brief Vérifie si un mouvement SEULTOU est possible
*\param game Etat du jeu
*\param r0 Ligne de départ
*\param c0 Colonne de départ
*\param r Ligne intermédiaire
*\param c Colonne intermédiaire
*\param er Ligne de l'ennemi
*\param ec Colonne de l'ennemi
*\return true si SEULTOU est possible, false sinon
*/


// Vérifie si un mouvement déclenche légalement l'effet SEULTOU.
bool game_can_seultou(const GameState *game, int r0, int c0, int r, int c, int er, int ec) {
    if (!game) return false;
    if (!game_in_bounds(er, ec)) return false;
    if (!game_in_bounds(r, c)) return false;

    Piece src = game->board[r0][c0].piece;
    Player me = game_piece_owner(src);
    Player opp = (me == PLAYER_BLUE) ? PLAYER_RED : PLAYER_BLUE;

    if (game_piece_owner(game->board[er][ec].piece) != opp) return false;

    int dr = (er > r) ? 1 : (er < r ? -1 : 0);
    int dc = (ec > c) ? 1 : (ec < c ? -1 : 0);

    int rr = er + dr;
    int cc = ec + dc;
    if (!game_in_bounds(rr, cc)) return true;

    Piece behind = game->board[rr][cc].piece;
    if (behind == PIECE_NONE) return true;
    if (game_piece_owner(behind) == me) return true;
    return false;
}


/**
*\fn bool game_apply_seultou(GameState *game, int r0, int c0, int r, int c, int er, int ec) 
*\brief Applique un mouvement SEULTOU
*\param game Etat du jeu
*\param r0 Ligne de départ
*\param c0 Colonne de départ
*\param r LIgne intermédiaire
*\param c Colonne Intermédiare
*\param er Ligne de l'ennemi
*\param ec Colonne de l'ennemi
*\return true si SEULTOU est appliqué, false sinon
*/


// Effectue un déplacement SEULTOU et résout ses effets secondaires.

bool game_apply_seultou(GameState *game, int r0, int c0, int r, int c, int er, int ec) {
    if (!game_can_seultou(game, r0, c0, r, c, er, ec)) return false;

    Piece mover_piece = game->board[r0][c0].piece;
    Player mover = game_piece_owner(mover_piece);
    Piece victim = game->board[er][ec].piece;

    game->board[r0][c0].piece = PIECE_NONE;
    game->board[r][c].piece = mover_piece;
    game->board[er][ec].piece = PIECE_NONE;
    game->ctrl[er][ec] = CTRL_NONE;

    if (game->board[r][c].cell != CELL_CITY_BLUE &&
        game->board[r][c].cell != CELL_CITY_RED) {
        game->ctrl[r][c] = owner_to_ctrl(mover);
    }

    char from[8], stop[8], enemy[8];
    game_coord_to_str(r0, c0, from, sizeof from);
    game_coord_to_str(r,  c,  stop, sizeof stop);
    game_coord_to_str(er, ec, enemy, sizeof enemy);
    const char *who = (mover == PLAYER_BLUE) ? "Joueur Bleu" : "Joueur Rouge";
    printf("%s (SEULTOU) : %s -> %s | ennemi retiré en %s.\n", who, from, stop, enemy);

    if ((mover_piece == PIECE_KING_BLUE || mover_piece == PIECE_KING_RED) &&
        is_opponent_city(game, mover, r, c)) {
        game->pending_victory = true;
        game->pending_winner = mover;
        snprintf(game->pending_reason, sizeof(game->pending_reason),
                 "Roi sur le camp adverse");
    }

    if (victim == PIECE_KING_BLUE || victim == PIECE_KING_RED) {
        game->pending_victory = true;
        game->pending_winner = mover;
        snprintf(game->pending_reason, sizeof(game->pending_reason),
                 "Exécution du roi (SEULTOU)");
    }

    int ncap = game_apply_linca(game, r, c);
    if (ncap > 0)
        printf("=> %d capture(s) LINCA effectuée(s).\n", ncap);

    if (!game->game_over)
        check_king_plus_one_rule(game);

    return true;
}


/**
*\fn void game_update_scores(GameState *game)
*\brief Met à jour les scores des joueurs
*\param game Etat du jeu
*/


// Recalcule les scores basés sur le contrôle et les unités restantes.

void game_update_scores(GameState *game) {
    if (!game) return;
    int blue_cells = 0, red_cells = 0;
    int blue_soldiers = 0, red_soldiers = 0;

    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            if (game->ctrl[r][c] == CTRL_BLUE) blue_cells++;
            else if (game->ctrl[r][c] == CTRL_RED) red_cells++;

            switch (game->board[r][c].piece) {
                case PIECE_SOLDIER_BLUE: blue_soldiers++; break;
                case PIECE_SOLDIER_RED:  red_soldiers++; break;
                default: break;
            }
        }
    }

    game->score_blue = blue_cells + blue_soldiers;
    game->score_red  = red_cells  + red_soldiers;
}


/**
*\fn void game_switch_turn(GameState *game)
*\brief Passe au tour suivant
*\param game Etat du jeu
*/


// Fait progresser le tour de jeu et alterne les joueurs actifs.

void game_switch_turn(GameState *game) {
    if (!game) return;
    if (game->turns_left > 0) game->turns_left--;
    game->turn = (game->turn == PLAYER_BLUE) ? PLAYER_RED : PLAYER_BLUE;
}


/**
*\fn bool game_check_victory_conditions(GameState *game)
*\brief Vérifie les conditions de victoire
*\param game Etat du jeu
*\return true si une condition de victoire est remplie, false sinon
*/

// Détecte et verrouille les états de victoire ou d'égalité.

bool game_check_victory_conditions(GameState *game) {
    if (!game) return false;

    if (game->pending_victory) {
        game->game_over = true;
        return true;
    }

    if (game->turns_left == 0) {
        game_update_scores(game);
        if (game->score_blue > game->score_red) {
            game->pending_victory = true;
            game->pending_winner = PLAYER_BLUE;
            snprintf(game->pending_reason, sizeof(game->pending_reason),
                     "Limite de tours atteinte");
        } else if (game->score_red > game->score_blue) {
            game->pending_victory = true;
            game->pending_winner = PLAYER_RED;
            snprintf(game->pending_reason, sizeof(game->pending_reason),
                     "Limite de tours atteinte");
        } else {
            game->pending_victory = true;
            game->pending_winner = PLAYER_NONE;
            snprintf(game->pending_reason, sizeof(game->pending_reason),
                     "Limite de tours atteinte");
        }
        game->game_over = true;
        return true;
    }

    return false;
}


/**
*\fn void game_get_legal_moves(const GameState *game, int r, int c, int moves[][2], int *count)
*\brief Récupère les mouvements légaux pour une pièce
*\param game Etat du jeu
*\param r Ligne de la pièce
*\param c Colonne de la pièce
*\param moves Tableau pour stocker les mouvements
*\param count Compteur de mouvements
*/

// Recense les destinations valides pour une pièce sélectionnée.
void game_get_legal_moves(const GameState *game, int r, int c, int moves[][2], int *count) {
    if (!moves || !count) return;
    *count = 0;
    if (!game || !game_in_bounds(r, c)) return;
    if (game_piece_owner(game->board[r][c].piece) != game->turn) return;

    for (int nc = 0; nc < BOARD_SIZE; ++nc) {
        if (nc == c) continue;
        if (game_can_move(game, r, c, r, nc)) {
            moves[*count][0] = r;
            moves[*count][1] = nc;
            (*count)++;
        }
    }

    for (int nr = 0; nr < BOARD_SIZE; ++nr) {
        if (nr == r) continue;
        if (game_can_move(game, r, c, nr, c)) {
            moves[*count][0] = nr;
            moves[*count][1] = c;
            (*count)++;
        }
    }
}

// ============================================================================
// Gestion de l'interface et du cycle de jeu
// ============================================================================

/**
*\struct AppContext
*\brief Contexte principal de l'application
*/


typedef struct { /**Etat du jeu */
    GameState game; /** Etat de l'interface utilisateur */
    UIState  *ui; /** Application GTK */
    GtkApplication *gtk_app; /** Mode de jeu */
    GameMode mode; /** IA activée */
    bool ia_enabled; /** Contexte client réseau */
    ClientContext client_ctx; /**Contexte serveur réseau */
    ServerContext server_ctx; /** Réseau initialisé */
    bool network_initialised; /** En attente d'un coup distant */
    bool waiting_remote; /** IA en cours d'execution */
    bool ai_running;
} AppContext;


/**
*\fn static void refresh_full_ui(AppContext *app)
*\brief Synchronise l'affichage principal avec l'état du jeu
*\param app Contexte de l'application
*/

// Synchronise l'affichage principal avec l'état du jeu.
static void refresh_full_ui(AppContext *app) {
    ui_refresh_board(app->ui, &app->game);
    ui_refresh_scores(app->ui, app->game.score_blue, app->game.score_red);
    ui_refresh_panel(app->ui, &app->game);
}


/**
*\fn static bool is_local_player_turn(const AppContext *app)
*\brief Indique si c'est au joueur local de jouer selon le mode courant
*\param app Contexte de l'application
*\return true si c'est au joueur local de jouer, false sinon
*/

// Indique si c'est au joueur local de jouer selon le mode courant.
static bool is_local_player_turn(const AppContext *app) {
    if (!app) return false;
    switch (app->mode) {
        case MODE_LOCAL:  return true;
        case MODE_CLIENT: return app->game.turn == PLAYER_BLUE;
        case MODE_SERVER: return app->game.turn == PLAYER_RED;
        default:          return false;
    }
}

/**
*\fn static Player ai_controlled_player(const AppContext *app)
*\brief Détermine quel joueur est piloté par l'IA
*\param app contexte de l'application
*\return Le joueur controlé par l'IA
*/


// Détermine quel joueur est piloté par l'IA.
static Player ai_controlled_player(const AppContext *app) {
    if (!app || !app->ia_enabled) return PLAYER_NONE;
    switch (app->mode) {
        case MODE_LOCAL:  return PLAYER_RED;
        case MODE_CLIENT: return PLAYER_BLUE;
        case MODE_SERVER: return PLAYER_RED;
        default:          return PLAYER_NONE;
    }
}

/**
*\fn static bool ai_must_play(const AppContext *app)
*\brief Vérifie si l'IA doit proposer un coup immédiatement
*\param app Contexte de l'application
*\return true si l'IA doit jouer, false sinon
*/

// Vérifie si l'IA doit proposer un coup immédiatement.
static bool ai_must_play(const AppContext *app) {
    Player ai = ai_controlled_player(app);
    if (ai == PLAYER_NONE) return false;
    if (!app || app->game.game_over) return false;
    return app->game.turn == ai;
}


/**
*\fn void maybe_start_ai(AppContext *app)
*\brief Démarre un calcul IA asynchrone lorsque c'est possible
*\param app Contexte de l'application
*/
static void maybe_start_ai(AppContext *app);


/**
*\fn static void update_turn_ui(AppContext *app)
*\brief Met à jour les messages d'etat et la sensibilité de l'interface
*\param app Contexte de l'application
*/

// Met à jour les messages d'état et la sensibilité de l'interface.
static void update_turn_ui(AppContext *app) {
    if (!app) return;
    if (app->game.game_over) {
        ui_set_status(app->ui, "Partie terminée");
        ui_set_board_sensitive(app->ui, false);
        return;
    }

    if (ai_must_play(app)) {
        ui_set_status(app->ui, "L'IA réfléchit...");
        ui_set_board_sensitive(app->ui, false);
        return;
    }

    if (app->mode == MODE_LOCAL) {
        char status[128];
        snprintf(status, sizeof status, "Tour de %s", game_player_name(app->game.turn));
        ui_set_status(app->ui, status);
        ui_set_board_sensitive(app->ui, true);
    } else if (is_local_player_turn(app)) {
        ui_set_status(app->ui, "À vous de jouer");
        ui_set_board_sensitive(app->ui, true);
    } else {
        ui_set_status(app->ui, "En attente de l'adversaire…");
        ui_set_board_sensitive(app->ui, false);
    }
}

/**
*\struct AIThreadArgs
*\brief Arguments pour le thread de l'IA
*/

typedef struct {
    AppContext *app; /**< Contexte de l'application */
} AIThreadArgs;

/**
*\struct AIResultTask
*\brief Résultat du calcul de l'IA
*/
typedef struct {
    AppContext *app;/**< Contexte de l'application */
    AI_Move     move; /**< Coup choisi par l'IA */
    bool        has_move; /**< Indique si un coup a été trouvé */
    Player      ai_player; /**< Joueur contrôlé par l'IA */
} AIResultTask;

/**
*\struct NetworkThreadArgs
*\brief Arguments pour les threads réseau
*/
typedef struct {
    AppContext *app; /**< Contexte de l'application */
} NetworkThreadArgs;

/**
*\struct RemoteMoveTask
*\brief Tâche de mouvement distant
*/
typedef struct {
    AppContext *app; /**< Contexte de l'application */
    char move[5]; /**< Message de mouvement (4caractères +null) */
} RemoteMoveTask;

/**
*\fn static void start_wait_for_remote(AppContext *app);
*\brief Lance un thread d'écoute réseau si necessaire
*\param app Contexte de l'application
*/
static void start_wait_for_remote(AppContext *app);

/**
*\fn static gboolean apply_ai_move_callback(gpointer data);
*\brief Callback exécuté dans le thread principal pour appliquer le coup IA
*\param data Données de callback
*\return FALSE pour indiquer que le callback est terminé 
*/
static gboolean apply_ai_move_callback(gpointer data);

/**
*\fn static void *ai_thread_func(void *arg);
*\brief Thread IA : calcule un coup optimal et programme son application
*\param arg Arguments du thread
*\return NULL
*/
static void *ai_thread_func(void *arg);


/**
*\fn static void clear_selection(AppContext *app)
*\brief Réinitialise la sélection courante sur le plateau UI
*\param app Contexte de l'application
*/

// Réinitialise la sélection courante sur le plateau UI.
static void clear_selection(AppContext *app) {
    app->game.have_selection = false;
    app->game.sel_r = app->game.sel_c = -1;
    ui_clear_selection(app->ui);
    ui_clear_move_hints(app->ui);
}


/**
*\fn static void apply_victory_ui(AppContext *app)
*\brief Affiche l'écran de résultat et bloque les interactions
*\param app Contexte de l'application
*/

// Affiche l'écran de résultat et bloque les interactions.
static void apply_victory_ui(AppContext *app) {
    if (app->game.pending_winner == PLAYER_NONE)
        ui_show_draw(app->ui, app->game.pending_reason);
    else
        ui_show_victory(app->ui, app->game.pending_winner, app->game.pending_reason);
    ui_set_board_sensitive(app->ui, false);
    ui_set_status(app->ui, "Partie terminée");
    app->waiting_remote = false;
#ifdef TEST
    force_quit_gtk();
#endif
}

/**
*\fn static void handle_turn_transition(AppContext *app)
*\brief Bascule sur le tour suivant et déclenche l'IA si besoin
*\param app Contexte de l'application 
*/

// Bascule sur le tour suivant et déclenche l'IA si besoin.
static void handle_turn_transition(AppContext *app) {
    game_switch_turn(&app->game);
    game_update_scores(&app->game);
    refresh_full_ui(app);
    update_turn_ui(app);
    maybe_start_ai(app);
}


/**
*\fn static void send_move_message(AppContext *app, int r0, int c0, int r1, int c1)
*\brief Envoie un coup joué au pair réseau actif
*\param app Contexte de l'application
*\param r0 Ligne de départ
*\param c0 Colonne de départ 
*\param r1 Ligne d'arrivée
*\param c1 Colonne d'arrivée
*/

// Envoie un coup jouée au pair réseau actif.
static void send_move_message(AppContext *app, int r0, int c0, int r1, int c1) {
    if (!app) return;
    char start[4];
    char end[4];
    char msg[8];
    game_coord_to_str(r0, c0, start, sizeof start);
    game_coord_to_str(r1, c1, end, sizeof end);
    snprintf(msg, sizeof(msg), "%s%s", start, end);

    if (app->mode == MODE_CLIENT) {
        client_send_message(&app->client_ctx, msg);
    } else if (app->mode == MODE_SERVER) {
        send_message(&app->server_ctx, msg);
    }
}


/**
*\fn static bool parse_move_string( const char *msg, int *r0,int *c0, int *r1, int *c1)
*\brief Convertit un message réseau en coordonnées exploitables
*\param msg Message réseau à parser
*\param r0 Pointeur pour stocker la ligne de départ
*\param c0 Pointeur pour stocker la colonne de départ
*\param r1 Pointeur pour stocker la ligne d'arrivée
*\param c1 Pointeur pour stocker la colonne d'arrivée
*\return true si la conversion réussit, false sinon
*/

// Convertit un message réseau en coordonnées exploitables.
static bool parse_move_string(const char *msg, int *r0, int *c0, int *r1, int *c1) {
    if (!msg) return false;
    char start[4] = {0};
    char dest[4]  = {0};
    if (strlen(msg) < 4) return false;
    start[0] = msg[0];
    start[1] = msg[1];
    dest[0]  = msg[2];
    dest[1]  = msg[3];
    if (!game_str_to_coord(start, r0, c0)) return false;
    if (!game_str_to_coord(dest,  r1, c1)) return false;
    return true;
}


/**
*\fn static gboolean on_remote_move_received(gpointer data)
*\brief Traite l'arrivée d'un coup distant et met à jour la partie
*\param data Données de callback (RemoteMoveTask)
*\return FALSE pour indiquer que le callback est terminé
*/

// Traite l'arrivée d'un coup distant et met à jour la partie.
static gboolean on_remote_move_received(gpointer data) {
    RemoteMoveTask *task = (RemoteMoveTask*)data;
    AppContext *app = task->app;

    int r0, c0, r1, c1;
    if (!parse_move_string(task->move, &r0, &c0, &r1, &c1)) {
        fprintf(stderr, "Coup réseau invalide : %s\n", task->move);
        app->waiting_remote = false;
        g_free(task);
        return FALSE;
    }

    app->waiting_remote = false;

    if (!game_apply_move(&app->game, r0, c0, r1, c1)) {
        fprintf(stderr, "Impossible d'appliquer le coup réseau %s\n", task->move);
        app->waiting_remote = false;
        g_free(task);
        return FALSE;
    }

    game_update_scores(&app->game);
    refresh_full_ui(app);

    if (game_check_victory_conditions(&app->game)) {
        apply_victory_ui(app);
    } else {
        handle_turn_transition(app);
        if (app->mode != MODE_LOCAL && !ai_must_play(app))
            start_wait_for_remote(app);
    }

    g_free(task);
    return FALSE;
}


/**
*\fn static gboolean on_remote_connection_lost(gpointer data)
*\brief Informe l'UI d'une coupure réseau détectée
*\param data Données de callback(AppContext)
*\return FALSE pour indiquer que le callback est terminée
*/


// Informe l'UI d'une coupure réseau détectée.
static gboolean on_remote_connection_lost(gpointer data) {
    AppContext *app = (AppContext*)data;
    fprintf(stderr, "Connexion réseau perdue.\n");
    ui_set_status(app->ui, "Connexion perdue");
    ui_set_board_sensitive(app->ui, false);
    app->waiting_remote = false;
    return FALSE;
}


/**
*\fn static void *wait_for_server_move_thread(void *arg)
*\brief Thread coté client : attend un coup venant du serveur
*\param arg Arguments du thread (NetworkThreadArgs)
*\return NULL
*/

// Thread côté client : attend un coup venant du serveur.
static void *wait_for_server_move_thread(void *arg) {
    NetworkThreadArgs *args = (NetworkThreadArgs*)arg;
    AppContext *app = args->app;
    g_free(args);

    char *msg = client_receive_message(&app->client_ctx);
    if (!msg) {
        g_idle_add(on_remote_connection_lost, app);
        return NULL;
    }

    RemoteMoveTask *task = g_new(RemoteMoveTask, 1);
    task->app = app;
    strncpy(task->move, msg, 4);
    task->move[4] = '\0';
    free(msg);

    g_idle_add(on_remote_move_received, task);
    return NULL;
}


/**
*\fn static void *wait_for_client_move_thread(void *arg)
*\brief Thread coté client : attend un coup venant du serveur
*\param arg Arguments du thread (NetworkThreadArgs)
*\return NULL
*/

// Thread côté serveur : attend un coup envoyé par le client.
static void *wait_for_client_move_thread(void *arg) {
    NetworkThreadArgs *args = (NetworkThreadArgs*)arg;
    AppContext *app = args->app;
    g_free(args);

    char *msg = receive_message(&app->server_ctx);
    if (!msg) {
        g_idle_add(on_remote_connection_lost, app);
        return NULL;
    }

    RemoteMoveTask *task = g_new(RemoteMoveTask, 1);
    task->app = app;
    strncpy(task->move, msg, 4);
    task->move[4] = '\0';
    free(msg);

    g_idle_add(on_remote_move_received, task);
    return NULL;
}


/**
*\fn static void start_wait_for_remote(AppContext *app)
*\brief Lance un thread d'ecoute réseau si necessaire
*\param app Contexte de l'application
*/

// Lance un thread d'écoute réseau si nécessaire.
static void start_wait_for_remote(AppContext *app) {
    if (!app) return;
    if (app->mode == MODE_LOCAL) return;
    if (app->waiting_remote) return;
    app->waiting_remote = true;

    NetworkThreadArgs *args = g_new(NetworkThreadArgs, 1);
    args->app = app;
    pthread_t tid;
    if (app->mode == MODE_CLIENT) {
        pthread_create(&tid, NULL, wait_for_server_move_thread, args);
    } else {
        pthread_create(&tid, NULL, wait_for_client_move_thread, args);
    }
    pthread_detach(tid);
}



/**
*\fn static gboolean apply_ai_move_callback(gpointer data)
*\brief Callback exécuté dans le thread principal pour appliquer le coup IA
*\param data Données de callback (AIResultTask)
*\return False pour indiquer que le callback est terminé
*/


// Callback exécuté dans le thread principal pour appliquer le coup IA.

static gboolean apply_ai_move_callback(gpointer data) {
    AIResultTask *task = (AIResultTask*)data;
    AppContext *app = task->app;

    app->ai_running = false;

    if (app->game.game_over || !ai_must_play(app)) {
        update_turn_ui(app);
        g_free(task);
        return FALSE;
    }

    if (!task->has_move) {
        Player opponent = (task->ai_player == PLAYER_BLUE) ? PLAYER_RED : PLAYER_BLUE;
        app->game.pending_victory = true;
        app->game.pending_winner = opponent;
        snprintf(app->game.pending_reason, sizeof(app->game.pending_reason), "IA ne peut plus jouer");
        apply_victory_ui(app);
        g_free(task);
        return FALSE;
    }

    int r0 = task->move.r0;
    int c0 = task->move.c0;
    int r1 = task->move.r1;
    int c1 = task->move.c1;

    if (!game_apply_move(&app->game, r0, c0, r1, c1)) {
        Player opponent = (task->ai_player == PLAYER_BLUE) ? PLAYER_RED : PLAYER_BLUE;
        fprintf(stderr, "IA: coup invalide (%d,%d)->(%d,%d)\n", r0, c0, r1, c1);
        app->game.pending_victory = true;
        app->game.pending_winner = opponent;
        snprintf(app->game.pending_reason, sizeof(app->game.pending_reason), "IA ne peut plus jouer");
        apply_victory_ui(app);
        g_free(task);
        return FALSE;
    }

    game_update_scores(&app->game);
    refresh_full_ui(app);

    if (app->mode != MODE_LOCAL)
        send_move_message(app, r0, c0, r1, c1);

    if (game_check_victory_conditions(&app->game)) {
        apply_victory_ui(app);
    } else {
        handle_turn_transition(app);
        if (app->mode != MODE_LOCAL && !ai_must_play(app))
            start_wait_for_remote(app);
    }

    g_free(task);
    return FALSE;
}


/**
*\fn static void *ai_thread_func(void *arg)
*\brief Thread IA : calcule un coup optimal et programme son application
*\param arg Arguments du thread (AIThreadArgs)
*\return NULL
*/

// Thread IA : calcule un coup optimal et programme son application.

static void *ai_thread_func(void *arg) {
    AIThreadArgs *args = (AIThreadArgs*)arg;
    AppContext *app = args->app;
    g_free(args);

    GameState snapshot = app->game;
    Square board[BOARD_SIZE][BOARD_SIZE];
    appstate_to_ai_board(snapshot.board, board);

    Player ai_player = ai_controlled_player(app);
    AI_Move best = {0};
    bool has_move = ai_choose_best_move_from_board(board, (AI_Player)ai_player, &best);

    AIResultTask *task = g_new(AIResultTask, 1);
    task->app = app;
    task->move = best;
    task->has_move = has_move;
    task->ai_player = ai_player;

    g_idle_add(apply_ai_move_callback, task);
    return NULL;
}


/**
*\fn static void maybe_start_ai(AppContext *app)
*\brief Démarre un calcul IA asynchrone lorsque c'est opptimal
*\param app Contexte de l'application 
*/

// Démarre un calcul IA asynchrone lorsque c'est opportun.
static void maybe_start_ai(AppContext *app) {
    if (!ai_must_play(app)) return;
    if (app->ai_running) return;
    app->ai_running = true;
    ui_set_status(app->ui, "L'IA réfléchit...");
    ui_set_board_sensitive(app->ui, false);

    AIThreadArgs *args = g_new(AIThreadArgs, 1);
    args->app = app;
    pthread_t tid;
    pthread_create(&tid, NULL, ai_thread_func, args);
    pthread_detach(tid);
}


/**
*\fn static bool on_ui_cell_clicked(void *user_data, int r,int c)
*\brief Gestionnaire de clic sur une case du plateau
*\param user_data Données utilisateur (AppContext)
*\param r Ligne de la case cliquée
*\param c Colonne de la case cliquée
*\return true si le clic a été traité, false sinon
*/

// Gestionnaire de clic sur une case du plateau.

static bool on_ui_cell_clicked(void *user_data, int r, int c) {
    AppContext *app = (AppContext*)user_data;
    if (app->game.game_over) return false;

    if (!is_local_player_turn(app)) return false;
    if (ai_must_play(app)) return false;

    if (!app->game.have_selection) {
        if (game_piece_owner(app->game.board[r][c].piece) == app->game.turn) {
            app->game.have_selection = true;
            app->game.sel_r = r;
            app->game.sel_c = c;
            ui_show_selection(app->ui, r, c);
            ui_show_move_hints(app->ui, &app->game, r, c);
            return true;
        }
        return false;
    }

    if (app->game.sel_r == r && app->game.sel_c == c) {
        clear_selection(app);
        return true;
    }

    int r0 = app->game.sel_r;
    int c0 = app->game.sel_c;
    bool moved = game_apply_move(&app->game, r0, c0, r, c);
    clear_selection(app);
    if (!moved) {
        ui_set_status(app->ui, "Coup invalide");
        return false;
    }

    game_update_scores(&app->game);
    refresh_full_ui(app);

    if (app->mode != MODE_LOCAL)
        send_move_message(app, r0, c0, r, c);

    if (game_check_victory_conditions(&app->game)) {
        apply_victory_ui(app);
    } else {
        handle_turn_transition(app);
        if (app->mode != MODE_LOCAL && !ai_must_play(app))
            start_wait_for_remote(app);
    }

    return true;
}


/**
*\fn static void on_ui_replay (void *user_data)
*\brief Réinitialise une partie depuis l'interface utilisateur
*\param user_data Données utilisateur (AppContext)
*/

// Réinitialise une partie depuis l'interface utilisateur.

static void on_ui_replay(void *user_data) {
    AppContext *app = (AppContext*)user_data;
    game_reset(&app->game);
    clear_selection(app);
    ui_hide_victory_overlay(app->ui);
    refresh_full_ui(app);
    app->waiting_remote = false;
    update_turn_ui(app);
    maybe_start_ai(app);
    if (app->mode == MODE_SERVER && app->game.turn == PLAYER_BLUE)
        start_wait_for_remote(app);
}


/**
*\fn static void on_app_activate(GtkApplication *gtk_app, gpointer user_data)
*\brief Callback GTK d'initialisation de la fenêtre principale
*\param gtk_app Application GTK
*\param user_data Données utilisateur (AppContext)
*/

// Callback GTK d'initialisation de la fenêtre principale.
static void on_app_activate(GtkApplication *gtk_app, gpointer user_data) {
    AppContext *app = (AppContext*)user_data;
    app->gtk_app = gtk_app;

    ui_load_css();
    GtkWindow *win = ui_get_window(app->ui);
    gtk_window_set_application(win, gtk_app);
    refresh_full_ui(app);
    update_turn_ui(app);
    maybe_start_ai(app);
    if (app->mode == MODE_SERVER && app->game.turn == PLAYER_BLUE)
        start_wait_for_remote(app);

    gtk_window_present(win);

#ifdef TEST
    if (app->mode == MODE_LOCAL) force_quit_gtk();
#endif
}


/**
*\fn static int run_application(AppContext *app, const char *app_id)
*\brief Crée l'application GTK et lance la boucle principale
*\param app Contexte de l'application
*\param app_id Identifiant de l'application
*\return Code de retour de l'application
*/


// Crée l'application GTK et lance la boucle principale.
static int run_application(AppContext *app, const char *app_id) {
    GtkApplication *gtk_app = gtk_application_new(app_id, G_APPLICATION_FLAGS_NONE);
    app->gtk_app = gtk_app;
    g_app_forced_quit = gtk_app;

    ui_set_callbacks(app->ui, (UICallbacks){
        .on_cell_clicked = on_ui_cell_clicked,
        .on_replay_requested = on_ui_replay,
        .game_data = app
    });

    g_signal_connect(gtk_app, "activate", G_CALLBACK(on_app_activate), app);

    int status = g_application_run(G_APPLICATION(gtk_app), 0, NULL);

    g_object_unref(gtk_app);
    app->gtk_app = NULL;
    return status;
}


/**
*\fn static int launch_game(AppContext *app, const char *app_id)
*\brief Prépare l'état du jeu puis lance l'interface graphique
*\param app Contexte de l'application
*\param app_id Identifiant de l'application
*\return Code de retour de l'application
*/


// Prépare l'état du jeu puis lance l'interface graphique.
static int launch_game(AppContext *app, const char *app_id) {
    game_init(&app->game, app->mode);
    app->ui = ui_create(app->mode, app->ia_enabled);
    if (!app->ui) {
        fprintf(stderr, "Impossible de créer l'interface utilisateur\n");
        return -1;
    }

    int status = run_application(app, app_id);

    ui_destroy(app->ui);
    app->ui = NULL;
    return status;
}

// ============================================================================
// Fonctions utilitaires historiques (CLI)
// ============================================================================

/**
*\fn int is_number(const char *s)
*\brief Vérifie qu'une chaine est strictement composée de chiffres
*\param s chaine à verifier
*\return 1 si la chaine est numerique, 0 sinon
*/

// Vérifie qu'une chaîne est strictement composée de chiffres.
int is_number(const char *s) {
    if (!s || !*s) return 0;
    for (const char *p = s; *p; ++p)
        if (*p < '0' || *p > '9')
            return 0;
    return 1;
}


/**
*\fn int validate_ip(const char *ip)
*\brief Controle basiquement le format d'une adresse IPv4
*\param ip Adresse IP à valider
*\return 1 si l'adresse est valide, 0 sinon
*/


// Contrôle basiquement le format d'une adresse IPv4.
int validate_ip(const char *ip) {
    if (!ip) return 0;
    int num, dots = 0;
    char *copy = strdup(ip);
    if (!copy) return 0;
    char *token = strtok(copy, ".");
    while (token) {
        if (!is_number(token)) {
            free(copy);
            return 0;
        }
        num = atoi(token);
        if (num < 0 || num > 255) {
            free(copy);
            return 0;
        }
        token = strtok(NULL, ".");
        dots++;
    }
    free(copy);
    return dots == 4;
}


/**
*\fn validate_ip_port(const char *arg)
*\brief Valide une chaine de type IP:Port 
*\param arg chaine à valider
*\return 1 si la chaine est valide, 0 sinon
*/

// Valide une chaîne de type IP:PORT.
int validate_ip_port(const char *arg) {
    if (!arg) return 0;
    char *copy = strdup(arg);
    if (!copy) return 0;
    char *sep = strchr(copy, ':');
    if (!sep) {
        free(copy);
        return 0;
    }
    *sep = '\0';
    char *ip = copy;
    char *port = sep + 1;
    int valid = validate_ip(ip) && is_number(port);
    free(copy);
    return valid;
}

// ============================================================================
// Fonctions play_* exposées
// ============================================================================


/**
*\fn play_local(_Bool ia_enabled)
*\brief Lance une partie en mode local
*\param ia_enabled true pour l'activer l'IA, false sinon
*\return Code de retour
*/

// Point d'entrée pour une partie locale.
int play_local(_Bool ia_enabled) {
    AppContext app = {0};
    app.mode = MODE_LOCAL;
    app.ia_enabled = ia_enabled;
    return launch_game(&app, "org.krojanty.local");
}


/**
*\fn int play_client(_Bool ia_enabled, char *ip, unsigned short port)
*\brief Lance une partie en mode client
*\param ia_enabled true pour activer l'IA, false sinon
*\param ip Adresse IP du serveur
*\param port Port du serveur
*\return Code de retour
*/

// Point d'entrée pour une partie en mode client réseau.
int play_client(_Bool ia_enabled, char *ip, unsigned short port) {
    AppContext app = {0};
    app.mode = MODE_CLIENT;
    app.ia_enabled = ia_enabled;

    client_init(&app.client_ctx, ip, port);
    int status = launch_game(&app, "org.krojanty.client");
    client_close(&app.client_ctx);
    return status;
}


/**
*\fn int play server(_Bool ia_enabled, unsigned short port)
*\brief Lance une partie en mode serveur
*\param ia_enabled true pour activer l'IA, false sinon
*\param port Port d'ecoute
*\return Code de retour
*/

// Point d'entrée pour héberger une partie en mode serveur.
int play_server(_Bool ia_enabled, unsigned short port) {
    AppContext app = {0};
    app.mode = MODE_SERVER;
    app.ia_enabled = ia_enabled;

#ifdef TEST
    if (port == 12345) {
        force_quit_gtk();
        return 0;   
    }
#endif

    server_init(&app.server_ctx, port);
    int status = launch_game(&app, "org.krojanty.server");
    close_server(&app.server_ctx);
    return status;
}
