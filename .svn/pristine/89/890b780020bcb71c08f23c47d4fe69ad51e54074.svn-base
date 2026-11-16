#ifndef AI_H
#define AI_H

#include "game.h"
#include <stdbool.h>

/**
 * @file ai.h
 * @brief Interface de l'Intelligence Artificielle pour le jeu Krojanty
 * @author yan.thenlot@uha.fr & sofiane.bouguerri@uha.fr & mael.lefebvre@uha.fr 
 * @note Documenté par becker.obie-obolo@uha.fr 
 * 
 
 */

#ifdef TEST
#define AI_MAX_DEPTH 1           /** Profondeur maximale de recherche réduite pour les tests */
#else
#define AI_MAX_DEPTH 3           /** Profondeur normale */
#endif
#define AI_MAX_MOVES 64          /** Nombre maximal de coups possibles par position*/
#define AI_INFINITY 1000000      /** Valeur infinie pour l'algorithme*/
#define AI_TRANSPOSITION_SIZE 65536  /** Taille de la table de transposition*/

/**
 * @enum AI_Player
 * @brief Représentation des joueurs pour l'IA
 * 
 * soit le joueur bleu pour la valeur 1 et le rouge pour la 2
 */
typedef enum {
    AI_PLAYER_BLUE = 1,
    AI_PLAYER_RED = 2
} AI_Player;

/**
 * @struct AI_Move
 * @brief Structure représentant un coup de l'IA
 */
typedef struct {
    int r0, c0;  /** Position de départ*/
    int r1, c1;  /** Position d'arrivée*/
    int score;   /** Score du coup (pour l'ordonnancement)*/
    bool is_seultou;     /** True si c'est un coup SEULTOU*/
    bool is_linca;       /** True si le coup génère des captures LINCA*/
    bool captures_king;  /** True si le coup capture le roi*/
    bool reaches_city;   /** True si le roi atteint la cité adverse*/
} AI_Move;

/**
 * @struct AI_GameState
 * @brief État de jeu simplifié pour l'IA
 * 
 * Cette structure est une abstraction du plateau complet, réduite aux informations 
 * essentielles pour la recherche et l’évaluation des coups. Elle permet à l’IA 
 * de manipuler plus efficacement l’état de la partie sans dépendre directement 
 * de l’interface ou d’autres structures plus lourdes.
 */
typedef struct {
    Square board[BOARD_SIZE][BOARD_SIZE]; /**< Plateau de jeu sous forme de matrice de cases. */
    AI_Player current_player; /**< Joueur courant (IA ou adversaire). */
    int blue_pieces;  /**< Nombre total de pièces bleues encore en jeu. */
    int red_pieces;  /**< Nombre total de pièces rouges encore en jeu. */
    int blue_kings;  /**< Nombre de rois bleus restants. */
    int red_kings;   /**< Nombre de rois rouges restants. */
    int turns_remaining;  /**< Nombre de tours restant avant la fin de la partie. */
    bool game_over;    /**< Indique si la partie est terminée. */
    int material_balance;  /**< Évaluation matérielle : différence de puissance entre les deux camps. */
    int position_score;  /**< Score positionnel : évaluation stratégique de la configuration actuelle. */
} AI_GameState;

/**
 * @struct AI_TranspositionEntry
 * @brief Entrée de la table de transposition
 * 
 * Chaque entrée stocke un état évalué du plateau pour éviter
 * de recalculer plusieurs fois les mêmes positions lors de la recherche.
 */
typedef struct {
    unsigned long long hash; /**< Clé de hachage Zobrist associée à la position. */
    int depth;  /**< Profondeur de recherche atteinte pour cette entrée. */
    int score;  /**< Score évalué pour la position. */
    AI_Move best_move; /** meilleur coup évalué par l'ia*/
    enum { EXACT, LOWER_BOUND, UPPER_BOUND } flag;  /**< Type de borne pour l’évaluation :- EXACT : score exact,
                                                        - LOWER_BOUND : borne inférieure,
                                                        - UPPER_BOUND : borne supérieure. */
} AI_TranspositionEntry;

/**
 * @struct AI_Context
 * @brief Contexte global de l'IA
 * 
 * Cette structure contient toutes les données persistantes nécessaires
 * au fonctionnement de l’IA : tables de transposition, hachages,
 * profondeur de recherche, statistiques.
 */
typedef struct {
    AI_TranspositionEntry transposition_table[AI_TRANSPOSITION_SIZE]; /**< Table de transposition. */
    unsigned long long zobrist_table[BOARD_SIZE][BOARD_SIZE][8];  /**< Table de hachage Zobrist pour générer les clés uniques. */
    int search_depth;  /**< Profondeur de recherche maximale configurée pour l’IA. */
    int nodes_searched;  /**< Nombre total de nœuds explorés dans l’arbre de recherche. */
    int alpha_beta_cutoffs;  /**< Nombre de coupures alpha-bêta effectuées. */
    AI_Player ai_player;  /**< Joueur contrôlé par l’IA (bleu ou rouge). */
    bool use_opening_book;  /**< Indique si l’IA doit utiliser un livre d’ouvertures. */
    bool endgame_mode;   /**< Indique si l’IA est en mode fin de partie. */
} AI_Context;

// ============================================================================
// Fonctions principales de l'IA
// ============================================================================

/**
 * @fn ai_init_context(AI_Context *ctx, AI_Player player)
 * @brief Initialise le contexte de l'IA
 * @param ctx Contexte à initialiser
 * @param player Joueur contrôlé par l'IA
 */
void ai_init_context(AI_Context *ctx, AI_Player player);

/**
 * @fn ai_choose_best_move_from_board(const Square board[BOARD_SIZE][BOARD_SIZE], AI_Player player, AI_Move *best_move)
 * @brief Choisit le meilleur coup à partir d'un plateau de jeu
 * @param board Plateau de jeu actuel
 * @param player Joueur qui doit jouer
 * @param best_move Pointeur vers le meilleur coup trouvé
 * @return true si un coup a été trouvé, false sinon
 */
bool ai_choose_best_move_from_board(const Square board[BOARD_SIZE][BOARD_SIZE], 
                                   AI_Player player, AI_Move *best_move);

/**
 * @fn appstate_to_ai_board(const Square game_board[BOARD_SIZE][BOARD_SIZE], Square ai_board[BOARD_SIZE][BOARD_SIZE])
 * @brief Convertit l'état du jeu en plateau pour l'IA
 * @param game_board Plateau du jeu
 * @param ai_board Plateau pour l'IA
 */
void appstate_to_ai_board(const Square game_board[BOARD_SIZE][BOARD_SIZE], 
                         Square ai_board[BOARD_SIZE][BOARD_SIZE]);

// ============================================================================
// Fonctions d'évaluation et d'analyse
// ============================================================================

/**
 * @fn ai_evaluate_position(AI_Context *ctx, const AI_GameState *state, AI_Player player)
 * @brief Évalue la position pour un joueur donné
 * @param ctx Contexte de l'IA
 * @param state État du jeu
 * @param player Joueur pour lequel évaluer
 * @return Score de la position (-AI_INFINITY à +AI_INFINITY)
 */
int ai_evaluate_position(AI_Context *ctx, const AI_GameState *state, AI_Player player);

/**
 * @fn ai_generate_moves(const AI_GameState *state, AI_Player player, AI_Move moves[AI_MAX_MOVES], int *move_count)
 * @brief Génère tous les coups légaux pour un joueur
 * @param state État du jeu
 * @param player Joueur
 * @param moves Array pour stocker les coups
 * @param move_count Pointeur vers le nombre de coups trouvés
 */
void ai_generate_moves(const AI_GameState *state, AI_Player player, 
                      AI_Move moves[AI_MAX_MOVES], int *move_count);

/**
 * @fn ai_order_moves(AI_Move moves[AI_MAX_MOVES], int move_count, const AI_GameState *state)
 * @brief Ordonne les coups par priorité tactique
 * @param moves Array de coups à ordonner
 * @param move_count Nombre de coups
 * @param state État du jeu
 */
void ai_order_moves(AI_Move moves[AI_MAX_MOVES], int move_count, 
                   const AI_GameState *state);

// ============================================================================
// Algorithme Minimax avec Alpha-Beta
// ============================================================================

/**
 * @fn ai_minimax_alpha_beta(AI_Context *ctx, AI_GameState *state, int depth, int alpha, int beta, bool maximizing_player, AI_Move *best_move)
 * @brief Algorithme Minimax avec élagage Alpha-Beta
 * @param ctx Contexte de l'IA
 * @param state État du jeu
 * @param depth Profondeur de recherche restante
 * @param alpha Valeur alpha
 * @param beta Valeur beta
 * @param maximizing_player True si c'est le tour du joueur maximisant
 * @param best_move Pointeur vers le meilleur coup
 * @return Score de la position
 */
int ai_minimax_alpha_beta(AI_Context *ctx, AI_GameState *state, int depth, 
                         int alpha, int beta, bool maximizing_player, 
                         AI_Move *best_move);

/**
 * @fn ai_quiescence_search(AI_Context *ctx, AI_GameState *state, int alpha, int beta, bool maximizing_player)
 * @brief Recherche quiescente pour les coups tactiques
 * @param ctx Contexte de l'IA
 * @param state État du jeu
 * @param alpha Valeur alpha
 * @param beta Valeur beta
 * @param maximizing_player True si c'est le tour du joueur maximisant
 * @return Score de la position
 */
int ai_quiescence_search(AI_Context *ctx, AI_GameState *state, 
                        int alpha, int beta, bool maximizing_player);

// ============================================================================
// Fonctions utilitaires
// ============================================================================

/**
 * @fn ai_apply_move(AI_GameState *state, const AI_Move *move)
 * @brief Applique un coup sur l'état du jeu
 * @param state État du jeu
 * @param move Coup à appliquer
 * @return true si le coup a été appliqué avec succès
 */
bool ai_apply_move(AI_GameState *state, const AI_Move *move);

/**
 * @fn ai_undo_move(AI_GameState *state, const AI_Move *move)
 * @brief Annule un coup sur l'état du jeu
 * @param state État du jeu
 * @param move Coup à annuler
 */
void ai_undo_move(AI_GameState *state, const AI_Move *move);

/**
 * @fn ai_is_tactical_move(const AI_GameState *state, const AI_Move *move)
 * @brief Vérifie si un coup est tactique (capture, échec, etc.)
 * @param state État du jeu
 * @param move Coup à vérifier
 * @return true si le coup est tactique
 */
bool ai_is_tactical_move(const AI_GameState *state, const AI_Move *move);

/**
 * @fn unsigned long long ai_zobrist_hash(const AI_Context *ctx, const AI_GameState *state)
 * @brief Calcule le hash Zobrist d'une position
 * @param ctx Contexte de l'IA
 * @param state État du jeu
 * @return Hash de la position
 */
unsigned long long ai_zobrist_hash(const AI_Context *ctx, const AI_GameState *state);

/**
 * @fn ai_is_endgame(const AI_GameState *state)
 * @brief Détecte si nous sommes en fin de partie
 * @param state État du jeu
 * @return true si c'est la fin de partie
 */
bool ai_is_endgame(const AI_GameState *state);

/**
 * @fn ai_convert_game_state(const GameState *game_state, AI_GameState *ai_state)
 * @brief Convertit un état de jeu GameState en AI_GameState
 * @param game_state État du jeu principal
 * @param ai_state État de l'IA à remplir
 */
void ai_convert_game_state(const GameState *game_state, AI_GameState *ai_state);

#endif // AI_H
