#ifndef AI_H
#define AI_H

#include "game.h"
#include <stdbool.h>

/**
 * @file ai.h
 * @brief Interface de l'Intelligence Artificielle pour le jeu Krojanty (v2)
 *
 * Moteur de recherche "negamax + alpha-beta" avec :
 *  - simulation EXACTE des règles de game.c (SEULTOU, LINCA, roi sur la cité,
 *    règle "roi + 1 soldat", contrôle de territoire, limite de tours),
 *  - approfondissement itératif borné par un temps de réflexion,
 *  - table de transposition Zobrist, killers, history, PVS, LMR,
 *  - recherche de quiescence sur les captures,
 *  - résolution exacte de la fin de partie (horizon = limite de tours).
 *
 * Fonction à appeler depuis game.c : ai_choose_best_move().
 */

#ifdef TEST
#define AI_MAX_DEPTH 3            /**< Profondeur max réduite pour les tests */
#define AI_TIME_LIMIT_MS 150      /**< Temps de réflexion réduit pour les tests */
#else
#define AI_MAX_DEPTH 40           /**< Profondeur max (le temps coupe bien avant) */
#define AI_TIME_LIMIT_MS 3000     /**< Temps de réflexion par coup (ms) */
#endif
#define AI_MAX_MOVES 192          /**< Nombre max de coups (10 tours x 16 destinations = 160) */
#define AI_INFINITY 1000000       /**< Valeur infinie */
#define AI_TRANSPOSITION_BITS 20
#define AI_TRANSPOSITION_SIZE (1 << AI_TRANSPOSITION_BITS) /**< Entrées de la table de transposition */

typedef enum {
    AI_PLAYER_BLUE = 1,
    AI_PLAYER_RED = 2
} AI_Player;

/** @brief Coup de l'IA (coordonnées ligne/colonne). */
typedef struct {
    int r0, c0;          /**< Départ */
    int r1, c1;          /**< Arrivée */
    int score;           /**< Score de recherche (point de vue du joueur qui joue) */
    bool is_seultou;     /**< Le coup déclenche un SEULTOU */
    bool is_linca;       /**< Le coup déclenche au moins une capture LINCA */
    bool captures_king;  /**< Le coup capture le roi adverse */
    bool reaches_city;   /**< Le roi atteint la cité adverse */
} AI_Move;

/** @brief État de jeu pour l'IA (copie légère de GameState). */
typedef struct {
    Square  board[BOARD_SIZE][BOARD_SIZE];
    Control ctrl[BOARD_SIZE][BOARD_SIZE];   /**< Territoire contrôlé (compte dans le score final) */
    AI_Player current_player;
    int blue_pieces, red_pieces;
    int blue_kings, red_kings;
    int turns_remaining;                    /**< Valeur de GameState.turns_left */
    bool game_over;
    int material_balance;
    int position_score;
} AI_GameState;

/** @brief Contexte persistant de l'IA. */
typedef struct {
    void *tt;                 /**< Table de transposition (allouée par ai_init_context) */
    int   search_depth;       /**< Profondeur maximale */
    int   time_limit_ms;      /**< Temps de réflexion par coup */
    int   nodes_searched;
    int   alpha_beta_cutoffs;
    int   last_depth;         /**< Profondeur atteinte au dernier coup */
    int   last_score;         /**< Score du dernier coup */
    AI_Player ai_player;
    bool  use_opening_book;   /**< (inutilisé, conservé pour compatibilité) */
    bool  endgame_mode;
} AI_Context;

/* ---------------------------------------------------------------------- */
/* API principale                                                         */
/* ---------------------------------------------------------------------- */

/** @brief Initialise le contexte (alloue la table de transposition). */
void ai_init_context(AI_Context *ctx, AI_Player player);

/** @brief Libère les ressources du contexte. */
void ai_free_context(AI_Context *ctx);

/**
 * @brief Choisit le meilleur coup pour le joueur dont c'est le tour dans @p game.
 *
 * Utilise board, ctrl, turn et turns_left : c'est la fonction à utiliser.
 * @return true si un coup existe.
 */
bool ai_choose_best_move(const GameState *game, AI_Move *best_move);

/** @brief Règle le temps de réflexion par coup (ms) : plus c'est long, plus l'IA est forte. */
void ai_set_time_limit(int ms);

/**
 * @brief Ancienne API (plateau seul). Le contrôle de territoire est déduit des
 *        pièces et la partie est supposée complète (64 tours) : moins précis.
 */
bool ai_choose_best_move_from_board(const Square board[BOARD_SIZE][BOARD_SIZE],
                                    AI_Player player, AI_Move *best_move);

void appstate_to_ai_board(const Square game_board[BOARD_SIZE][BOARD_SIZE],
                          Square ai_board[BOARD_SIZE][BOARD_SIZE]);

/** @brief Convertit un GameState en AI_GameState (utilise turn / turns_left). */
void ai_convert_game_state(const GameState *game_state, AI_GameState *ai_state);

/* ---------------------------------------------------------------------- */
/* Utilitaires (conservés pour compatibilité / tests)                      */
/* ---------------------------------------------------------------------- */

int  ai_evaluate_position(AI_Context *ctx, const AI_GameState *state, AI_Player player);
void ai_generate_moves(const AI_GameState *state, AI_Player player,
                       AI_Move moves[AI_MAX_MOVES], int *move_count);
bool ai_apply_move(AI_GameState *state, const AI_Move *move);
bool ai_is_tactical_move(const AI_GameState *state, const AI_Move *move);
bool ai_is_endgame(const AI_GameState *state);
unsigned long long ai_zobrist_hash(const AI_Context *ctx, const AI_GameState *state);

#endif // AI_H