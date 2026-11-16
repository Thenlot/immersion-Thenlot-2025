/**
 * @file ai.c
 * @brief Implémentation de l'Intelligence Artificielle pour le jeu Krojanty
 * @author yan.thanlou@uha.fr
 * @note Fichier documenté par hadil.abahmane@uha.fr
 * 
 * Cette IA utilise l'algorithme Minimax avec élagage Alpha-Beta,
 * une fonction d'évaluation sophistiquée, et des optimisations avancées
 * pour dominer au jeu Krojanty.
 * 
 * Caractéristiques principales :
 * - Algorithme Minimax avec élagage Alpha-Beta
 * - Table de transposition avec hachage Zobrist
 * - Recherche quiescente pour les coups tactiques
 * - Fonction d'évaluation multi-critères
 * - Détection spécialisée des règles SEULTOU et LINCA
 * - Stratégies d'ouverture et de fin de partie
 * - Ordonnancement intelligent des coups
 */

#include "ai.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

// ============================================================================
// Variables globales et contexte
// ============================================================================

static AI_Context g_ai_context = {0};
static bool g_context_initialized = false;

// Directions orthogonales (Nord, Sud, Ouest, Est)
static const int AI_DIRECTIONS[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

// ============================================================================
// Fonctions utilitaires internes
// ============================================================================

/**
 * \fn static unsigned long long random_uint64(void)
 * \brief Générateur de nombres aléatoires pour Zobrist
 * \return Nombre aléatoire 64 bits
 */
static unsigned long long random_uint64(void) {
    static bool seeded = false;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = true;
    }
    
    unsigned long long result = 0;
    for (int i = 0; i < 8; i++) {
        result = (result << 8) | (rand() & 0xFF);
    }
    return result;
}

/**
 * \fn static AI_Player player_to_ai_player(Player p)
 * \brief Convertir un Player en AI_Player
 * \param p Joueur à convertir
 * \return Joueur IA correspondant
 */
static AI_Player player_to_ai_player(Player p) {
    switch (p) {
        case PLAYER_BLUE: return AI_PLAYER_BLUE;
        case PLAYER_RED:  return AI_PLAYER_RED;
        default:          return AI_PLAYER_BLUE;
    }
}

/**
 * \fn static AI_Player ai_get_opponent(AI_Player player)
 * \brief Obtenir le joueur adverse
 * \param player Joueur actuel
 * \return Joueur adverse
 */
static AI_Player ai_get_opponent(AI_Player player) {
    return (player == AI_PLAYER_BLUE) ? AI_PLAYER_RED : AI_PLAYER_BLUE;
}

/**
 * \fn static bool ai_piece_belongs_to_player(Piece piece, AI_Player player)
 * \brief Vérifier si une pièce appartient à un joueur
 * \param piece Pièce à vérifier
 * \param player Joueur à tester
 * \return true si la pièce appartient au joueur, false sinon
 */
static bool ai_piece_belongs_to_player(Piece piece, AI_Player player) {
    Player owner = game_piece_owner(piece);
    return player_to_ai_player(owner) == player;
}

/**
 * \fn static bool ai_is_king(Piece piece)
 * \brief Vérifier si une pièce est un roi
 * \param piece Pièce à vérifier
 * \return true si c'est un roi, false sinon
 */
static bool ai_is_king(Piece piece) {
    return piece == PIECE_KING_BLUE || piece == PIECE_KING_RED;
}

/**
 * \fn static int ai_get_piece_value(Piece piece)
 * \brief Obtenir la valeur matérielle d'une pièce
 * \param piece Pièce à évaluer
 * \return Valeur matérielle de la pièce
 */
static int ai_get_piece_value(Piece piece) {
    switch (piece) {
        case PIECE_KING_BLUE:
        case PIECE_KING_RED:
            return 1000;  // Le roi vaut beaucoup
        case PIECE_SOLDIER_BLUE:
        case PIECE_SOLDIER_RED:
            return 100;   // Les soldats valent moins
        default:
            return 0;
    }
}

// ============================================================================
// Fonctions de conversion et d'interface
// ============================================================================

/**
 * \fn void appstate_to_ai_board(const Square game_board[BOARD_SIZE][BOARD_SIZE], Square ai_board[BOARD_SIZE][BOARD_SIZE])
 * \brief Convertir un plateau de jeu en plateau IA
 * \param game_board Plateau source
 * \param ai_board Plateau destination
 */
void appstate_to_ai_board(const Square game_board[BOARD_SIZE][BOARD_SIZE], 
                         Square ai_board[BOARD_SIZE][BOARD_SIZE]) {
    if (!game_board || !ai_board) return;
    
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            ai_board[r][c] = game_board[r][c];
        }
    }
}

/**
 * \fn void ai_convert_game_state(const GameState *game_state, AI_GameState *ai_state)
 * \brief Convertir un état de jeu en état IA
 * \param game_state État source
 * \param ai_state État destination
 */
void ai_convert_game_state(const GameState *game_state, AI_GameState *ai_state) {
    if (!game_state || !ai_state) return;
    
    // Copie du plateau
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            ai_state->board[r][c] = game_state->board[r][c];
        }
    }
    
    // Conversion du joueur actuel
    ai_state->current_player = player_to_ai_player(game_state->current_player);
    
    // Comptage des pièces
    ai_state->blue_pieces = 0;
    ai_state->red_pieces = 0;
    ai_state->blue_kings = 0;
    ai_state->red_kings = 0;
    
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            Piece piece = ai_state->board[r][c].piece;
            switch (piece) {
                case PIECE_KING_BLUE:
                    ai_state->blue_pieces++;
                    ai_state->blue_kings++;
                    break;
                case PIECE_SOLDIER_BLUE:
                    ai_state->blue_pieces++;
                    break;
                case PIECE_KING_RED:
                    ai_state->red_pieces++;
                    ai_state->red_kings++;
                    break;
                case PIECE_SOLDIER_RED:
                    ai_state->red_pieces++;
                    break;
                default:
                    break;
            }
        }
    }
    
    ai_state->turns_remaining = game_state->turns_left;
    ai_state->game_over = game_state->game_over;
    ai_state->material_balance = ai_state->blue_pieces - ai_state->red_pieces;
    ai_state->position_score = 0; // Sera calculé par l'évaluation
}

// ============================================================================
// Initialisation et configuration
// ============================================================================

/**
 * \fn void ai_init_context(AI_Context *ctx, AI_Player player)
 * \brief Initialiser le contexte de l'IA
 * \param ctx Contexte à initialiser
 * \param player Joueur contrôlé par l'IA
 */
void ai_init_context(AI_Context *ctx, AI_Player player) {
    if (!ctx) return;
    
    memset(ctx, 0, sizeof(AI_Context));
    
    ctx->ai_player = player;
    ctx->search_depth = AI_MAX_DEPTH;
    ctx->use_opening_book = true;
    ctx->endgame_mode = false;
    
    // Initialisation de la table de hachage Zobrist
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            for (int piece = 0; piece < 8; piece++) {
                ctx->zobrist_table[r][c][piece] = random_uint64();
            }
        }
    }
    
    printf("IA initialisée pour le joueur %s\n", 
           (player == AI_PLAYER_BLUE) ? "Bleu" : "Rouge");
}

// ============================================================================
// Hachage Zobrist et table de transposition
// ============================================================================

/**
 * \fn unsigned long long ai_zobrist_hash(const AI_Context *ctx, const AI_GameState *state)
 * \brief Calculer le hash Zobrist d'un état de jeu
 * \param ctx Contexte IA
 * \param state État de jeu
 * \return Hash Zobrist 64 bits
 */
unsigned long long ai_zobrist_hash(const AI_Context *ctx, const AI_GameState *state) {
    if (!ctx || !state) return 0;
    
    unsigned long long hash = 0;
    
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            Piece piece = state->board[r][c].piece;
            if (piece != PIECE_NONE) {
                int piece_index = (int)piece - 1; // PIECE_NONE = 0, autres commencent à 1
                hash ^= ctx->zobrist_table[r][c][piece_index];
            }
        }
    }
    
    // Ajouter le joueur actuel au hash
    if (state->current_player == AI_PLAYER_RED) {
        hash ^= 0x123456789ABCDEF0ULL;
    }
    
    return hash;
}

/**
 * \fn static AI_TranspositionEntry* ai_probe_transposition(AI_Context *ctx, unsigned long long hash)
 * \brief Rechercher dans la table de transposition
 * \param ctx Contexte IA
 * \param hash Hash à rechercher
 * \return Entrée de transposition ou NULL si non trouvée
 */
static AI_TranspositionEntry* ai_probe_transposition(AI_Context *ctx, 
                                                    unsigned long long hash) {
    if (!ctx) return NULL;
    
    int index = hash % AI_TRANSPOSITION_SIZE;
    AI_TranspositionEntry *entry = &ctx->transposition_table[index];
    
    if (entry->hash == hash) {
        return entry;
    }
    
    return NULL;
}

/**
 * \fn static void ai_store_transposition(AI_Context *ctx, unsigned long long hash, int depth, int score, const AI_Move *best_move, int flag)
 * \brief Stocker une entrée dans la table de transposition
 * \param ctx Contexte IA
 * \param hash Hash de l'état
 * \param depth Profondeur de recherche
 * \param score Score évalué
 * \param best_move Meilleur coup trouvé
 * \param flag Type d'entrée (EXACT, LOWER_BOUND, UPPER_BOUND)
 */
static void ai_store_transposition(AI_Context *ctx, unsigned long long hash,
                                  int depth, int score, const AI_Move *best_move,
                                  int flag) {
    if (!ctx || !best_move) return;
    
    int index = hash % AI_TRANSPOSITION_SIZE;
    AI_TranspositionEntry *entry = &ctx->transposition_table[index];
    
    // Remplace seulement si la nouvelle entrée est plus profonde
    if (entry->hash == 0 || entry->depth <= depth) {
        entry->hash = hash;
        entry->depth = depth;
        entry->score = score;
        entry->best_move = *best_move;
        entry->flag = flag;
    }
}

// ============================================================================
// Génération et validation des coups
// ============================================================================

/**
 * \fn static bool ai_is_path_clear(const AI_GameState *state, int r0, int c0, int r1, int c1)
 * \brief Vérifier si un chemin orthogonal est libre
 * \param state État de jeu
 * \param r0 Ligne de départ
 * \param c0 Colonne de départ
 * \param r1 Ligne d'arrivée
 * \param c1 Colonne d'arrivée
 * \return true si le chemin est libre, false sinon
 */
static bool ai_is_path_clear(const AI_GameState *state, int r0, int c0, int r1, int c1) {
    if (!state) return false;
    if (!game_in_bounds(r0, c0) || !game_in_bounds(r1, c1)) return false;
    
    // Doit être un mouvement orthogonal
    if (r0 != r1 && c0 != c1) return false;
    
    // La destination doit être libre
    if (state->board[r1][c1].piece != PIECE_NONE) return false;
    
    // Vérifier le chemin
    if (r0 == r1) {
        // Mouvement horizontal
        int step = (c1 > c0) ? 1 : -1;
        for (int c = c0 + step; c != c1; c += step) {
            if (state->board[r0][c].piece != PIECE_NONE) {
                return false;
            }
        }
    } else {
        // Mouvement vertical
        int step = (r1 > r0) ? 1 : -1;
        for (int r = r0 + step; r != r1; r += step) {
            if (state->board[r][c0].piece != PIECE_NONE) {
                return false;
            }
        }
    }
    
    return true;
}

/**
 * \fn static bool ai_analyze_seultou(const AI_GameState *state, AI_Move *move, AI_Player player)
 * \brief Vérifier et analyse un coup SEULTOU
 * \param state État de jeu
 * \param move Coup à analyser
 * \param player Joueur effectuant le coup
 * \return true si c'est un SEULTOU valide, false sinon
 */
static bool ai_analyze_seultou(const AI_GameState *state, AI_Move *move, 
                              AI_Player player) {
    if (!state || !move) return false;
    
    // Direction du mouvement
    int dr = (move->r1 > move->r0) ? 1 : (move->r1 < move->r0 ? -1 : 0);
    int dc = (move->c1 > move->c0) ? 1 : (move->c1 < move->c0 ? -1 : 0);
    
    // Position de l'ennemi potentiel
    int er = move->r1 + dr;
    int ec = move->c1 + dc;
    
    if (!game_in_bounds(er, ec)) return false;
    
    Piece enemy_piece = state->board[er][ec].piece;
    if (enemy_piece == PIECE_NONE) return false;
    
    // Vérifier que c'est bien un ennemi
    AI_Player enemy_owner = player_to_ai_player(game_piece_owner(enemy_piece));
    if (enemy_owner == player) return false;
    
    // C'est un coup SEULTOU valide !
    move->is_seultou = true;
    if (ai_is_king(enemy_piece)) {
        move->captures_king = true;
        move->score += 10000; // Très prioritaire
    } else {
        move->score += 500; // Capture normale
    }
    
    return true;
}

/**
 * \fn static bool ai_analyze_linca(const AI_GameState *state, AI_Move *move, AI_Player player)
 * \brief Détecter les captures LINCA potentielles après un mouvement
 * \param state État de jeu
 * \param move Coup à analyser
 * \param player Joueur effectuant le coup
 * \return true si LINCA est détecté, false sinon
 */
static bool ai_analyze_linca(const AI_GameState *state, AI_Move *move, 
                            AI_Player player) {
    if (!state || !move) return false;
    
    bool has_linca = false;
    
    // Vérifier toutes les directions depuis la destination
    for (int d = 0; d < 4; d++) {
        int dr = AI_DIRECTIONS[d][0];
        int dc = AI_DIRECTIONS[d][1];
        
        int rm = move->r1 + dr;     // Position du milieu
        int cm = move->c1 + dc;
        int ra = move->r1 + 2 * dr; // Position après
        int ca = move->c1 + 2 * dc;
        
        if (!game_in_bounds(rm, cm) || !game_in_bounds(ra, ca)) continue;
        
        Piece mid_piece = state->board[rm][cm].piece;
        Piece after_piece = state->board[ra][ca].piece;
        
        if (mid_piece != PIECE_NONE && after_piece != PIECE_NONE) {
            AI_Player mid_owner = player_to_ai_player(game_piece_owner(mid_piece));
            AI_Player after_owner = player_to_ai_player(game_piece_owner(after_piece));
            
            // LINCA : ennemi au milieu, allié après
            if (mid_owner != player && after_owner == player) {
                has_linca = true;
                move->is_linca = true;
                
                if (ai_is_king(mid_piece)) {
                    move->captures_king = true;
                    move->score += 15000; // Capture du roi par LINCA
                } else {
                    move->score += 300; // Capture LINCA normale
                }
            }
        }
    }
    
    return has_linca;
}

/**
 * \fn static bool ai_check_city_capture(const AI_GameState *state, AI_Move *move, AI_Player player)
 * \brief Vérifier si le roi atteint la cité adverse
 * \param state État de jeu
 * \param move Coup à analyser
 * \param player Joueur effectuant le coup
 * \return true si la cité est atteinte, false sinon
 */
static bool ai_check_city_capture(const AI_GameState *state, AI_Move *move, 
                                 AI_Player player) {
    if (!state || !move) return false;
    
    Piece moving_piece = state->board[move->r0][move->c0].piece;
    if (!ai_is_king(moving_piece)) return false;
    
    CellKind target_cell = state->board[move->r1][move->c1].cell;
    
    bool reaches_enemy_city = false;
    if (player == AI_PLAYER_BLUE && target_cell == CELL_CITY_RED) {
        reaches_enemy_city = true;
    } else if (player == AI_PLAYER_RED && target_cell == CELL_CITY_BLUE) {
        reaches_enemy_city = true;
    }
    
    if (reaches_enemy_city) {
        move->reaches_city = true;
        move->score += 20000; // Victoire immédiate
        return true;
    }
    
    return false;
}
/**
 * \fn void ai_generate_moves(const AI_GameState *state, AI_Player player, AI_Move moves[AI_MAX_MOVES], int *move_count)
 * \brief Génèrer tous les coups possibles pour un joueur
 * \param state État de jeu
 * \param player Joueur pour lequel générer les coups
 * \param moves Tableau pour stocker les coups
 * \param move_count Compteur de coups générés
 */
void ai_generate_moves(const AI_GameState *state, AI_Player player, 
                      AI_Move moves[AI_MAX_MOVES], int *move_count) {
    if (!state || !moves || !move_count) return;
    
    *move_count = 0;
    
    for (int r0 = 0; r0 < BOARD_SIZE && *move_count < AI_MAX_MOVES; r0++) {
        for (int c0 = 0; c0 < BOARD_SIZE && *move_count < AI_MAX_MOVES; c0++) {
            Piece piece = state->board[r0][c0].piece;
            if (piece == PIECE_NONE) continue;
            if (!ai_piece_belongs_to_player(piece, player)) continue;
            
            // Essayer tous les mouvements orthogonaux possibles
            for (int r1 = 0; r1 < BOARD_SIZE && *move_count < AI_MAX_MOVES; r1++) {
                for (int c1 = 0; c1 < BOARD_SIZE && *move_count < AI_MAX_MOVES; c1++) {
                    if (r0 == r1 && c0 == c1) continue;
                    if (r0 != r1 && c0 != c1) continue; // Seulement orthogonal
                    
                    if (ai_is_path_clear(state, r0, c0, r1, c1)) {
                        AI_Move move = {
                            .r0 = r0, .c0 = c0, .r1 = r1, .c1 = c1,
                            .score = 0, .is_seultou = false, .is_linca = false,
                            .captures_king = false, .reaches_city = false
                        };
                        
                        // Vérifier SEULTOU
                        ai_analyze_seultou(state, &move, player);
                        
                        // Vérifier LINCA (si ce n'est pas déjà SEULTOU)
                        if (!move.is_seultou) {
                            ai_analyze_linca(state, &move, player);
                        }
                        
                        // Vérifier capture de cité
                        ai_check_city_capture(state, &move, player);
                        
                        moves[(*move_count)++] = move;
                    }
                }
            }
        }
    }
}

// ============================================================================
// Ordonnancement des coups
// ============================================================================
/**
 * \fn void ai_order_moves(AI_Move moves[AI_MAX_MOVES], int move_count, const AI_GameState *state)
 * \brief Ordonner les coups par score décroissant
 * \param moves Tableau de coups à ordonner
 * \param move_count Nombre de coups
 * \param state État de jeu (pour référence)
 */
void ai_order_moves(AI_Move moves[AI_MAX_MOVES], int move_count, 
                   const AI_GameState *state) {
    if (!moves || !state || move_count <= 1) return;
    
    // Tri par sélection basé sur le score
    for (int i = 0; i < move_count - 1; i++) {
        int max_idx = i;
        for (int j = i + 1; j < move_count; j++) {
            if (moves[j].score > moves[max_idx].score) {
                max_idx = j;
            }
        }
        if (max_idx != i) {
            AI_Move temp = moves[i];
            moves[i] = moves[max_idx];
            moves[max_idx] = temp;
        }
    }
}
/**
 * \fn bool ai_is_tactical_move(const AI_GameState *state, const AI_Move *move)
 * \brief Vérifier si un coup est tactique (capture ou victoire)
 * \param state État de jeu
 * \param move Coup à vérifier
 * \return true si le coup est tactique, false sinon
 */
bool ai_is_tactical_move(const AI_GameState *state, const AI_Move *move) {
    if (!state || !move) return false;
    
    return move->is_seultou || move->is_linca || 
           move->captures_king || move->reaches_city;
}

// ============================================================================
// Application et annulation de coups
// ============================================================================
/**
 * \fn bool ai_apply_move(AI_GameState *state, const AI_Move *move)
 * \brief Appliquer un coup à l'état de jeu
 * \param state État de jeu à modifier
 * \param move Coup à appliquer
 * \return true si le coup est appliqué avec succès, false sinon
 */
bool ai_apply_move(AI_GameState *state, const AI_Move *move) {
    if (!state || !move) return false;
    if (!game_in_bounds(move->r0, move->c0) || !game_in_bounds(move->r1, move->c1)) return false;
    
    Piece moving_piece = state->board[move->r0][move->c0].piece;
    if (moving_piece == PIECE_NONE) return false;
    
    // Mouvement de base
    state->board[move->r0][move->c0].piece = PIECE_NONE;
    state->board[move->r1][move->c1].piece = moving_piece;
    
    // Traitement SEULTOU
    if (move->is_seultou) {
        int dr = (move->r1 > move->r0) ? 1 : (move->r1 < move->r0 ? -1 : 0);
        int dc = (move->c1 > move->c0) ? 1 : (move->c1 < move->c0 ? -1 : 0);
        int er = move->r1 + dr;
        int ec = move->c1 + dc;
        
        if (game_in_bounds(er, ec)) {
            state->board[er][ec].piece = PIECE_NONE;
        }
    }
    
    // Traitement LINCA (simplifié pour l'IA)
    if (move->is_linca) {
        AI_Player player = player_to_ai_player(game_piece_owner(moving_piece));
        for (int d = 0; d < 4; d++) {
            int dr = AI_DIRECTIONS[d][0];
            int dc = AI_DIRECTIONS[d][1];
            
            int rm = move->r1 + dr;
            int cm = move->c1 + dc;
            int ra = move->r1 + 2 * dr;
            int ca = move->c1 + 2 * dc;
            
            if (!game_in_bounds(rm, cm) || !game_in_bounds(ra, ca)) continue;
            
            Piece mid_piece = state->board[rm][cm].piece;
            Piece after_piece = state->board[ra][ca].piece;
            
            if (mid_piece != PIECE_NONE && after_piece != PIECE_NONE) {
                AI_Player mid_owner = player_to_ai_player(game_piece_owner(mid_piece));
                AI_Player after_owner = player_to_ai_player(game_piece_owner(after_piece));
                
                if (mid_owner != player && after_owner == player) {
                    state->board[rm][cm].piece = PIECE_NONE;
                }
            }
        }
    }
    
    // Mise à jour du joueur actuel
    state->current_player = ai_get_opponent(state->current_player);
    
    return true;
}
/**
 * \fn void ai_undo_move(AI_GameState *state, const AI_Move *move)
 * \brief Annuler un coup (non implémenté complètement)
 * \param state État de jeu
 * \param move Coup à annuler
 */
void ai_undo_move(AI_GameState *state, const AI_Move *move) {
    // Implémentation simplifiée pour l'IA
    // Dans une version complète, il faudrait sauvegarder l'état avant application
    (void)state; (void)move;
}

// ============================================================================
// Fonction d'évaluation sophistiquée
// ============================================================================

/**
 * \fn static int ai_evaluate_material(const AI_GameState *state, AI_Player player)
 * \brief Évaluer la position matérielle
 * \param state État de jeu
 * \param player Joueur à évaluer
 * \return Score matériel
 */
static int ai_evaluate_material(const AI_GameState *state, AI_Player player) {
    if (!state) return 0;
    
    int my_value = 0;
    int enemy_value = 0;
    
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            Piece piece = state->board[r][c].piece;
            if (piece == PIECE_NONE) continue;
            
            int value = ai_get_piece_value(piece);
            
            if (ai_piece_belongs_to_player(piece, player)) {
                my_value += value;
            } else {
                enemy_value += value;
            }
        }
    }
    
    return my_value - enemy_value;
}

/**
 * \fn static int ai_evaluate_king_position(const AI_GameState *state, AI_Player player)
 * \brief Évaluer la position du roi
 * \param state État de jeu
 * \param player Joueur à évaluer
 * \return Score de position du roi
 */
static int ai_evaluate_king_position(const AI_GameState *state, AI_Player player) {
    if (!state) return 0;
    
    int score = 0;
    
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            Piece piece = state->board[r][c].piece;
            if (!ai_is_king(piece)) continue;
            
            if (ai_piece_belongs_to_player(piece, player)) {
                // Mon roi
                if (player == AI_PLAYER_BLUE) {
                    // Récompenser l'avancement vers la cité rouge (8,8)
                    score += (r + c) * 2;
                } else {
                    // Récompenser l'avancement vers la cité bleue (0,0)
                    score += (BOARD_SIZE - 1 - r + BOARD_SIZE - 1 - c) * 2;
                }
                
                // Pénaliser si le roi est trop isolé
                int allies_nearby = 0;
                for (int d = 0; d < 4; d++) {
                    int nr = r + AI_DIRECTIONS[d][0];
                    int nc = c + AI_DIRECTIONS[d][1];
                    if (game_in_bounds(nr, nc)) {
                        Piece neighbor = state->board[nr][nc].piece;
                        if (neighbor != PIECE_NONE && 
                            ai_piece_belongs_to_player(neighbor, player)) {
                            allies_nearby++;
                        }
                    }
                }
                if (allies_nearby == 0) score -= 50; // Roi isolé
                
            } else {
                // Roi ennemi - inverser l'évaluation
                if (player == AI_PLAYER_BLUE) {
                    score -= (BOARD_SIZE - 1 - r + BOARD_SIZE - 1 - c) * 2;
                } else {
                    score -= (r + c) * 2;
                }
            }
        }
    }
    
    return score;
}

/**
 * \fn static int ai_evaluate_mobility(const AI_GameState *state, AI_Player player)
 * \brief Évaluer la mobilité des pièces
 * \param state État de jeu
 * \param player Joueur à évaluer
 * \return Score de mobilité
 */
static int ai_evaluate_mobility(const AI_GameState *state, AI_Player player) {
    if (!state) return 0;
    
    AI_Move my_moves[AI_MAX_MOVES];
    AI_Move enemy_moves[AI_MAX_MOVES];
    int my_count, enemy_count;
    
    ai_generate_moves(state, player, my_moves, &my_count);
    ai_generate_moves(state, ai_get_opponent(player), enemy_moves, &enemy_count);
    
    return (my_count - enemy_count) * 10;
}
/**
 * \fn static int ai_evaluate_center_control(const AI_GameState *state, AI_Player player)
 * \brief Évaluer le contrôle du centre
 * \param state État de jeu
 * \param player Joueur à évaluer
 * \return Score de contrôle du centre
 */
static int ai_evaluate_center_control(const AI_GameState *state, AI_Player player) {
    if (!state) return 0;
    
    int score = 0;
    int center = BOARD_SIZE / 2;
    
    // Cases centrales importantes
    for (int r = center - 1; r <= center + 1; r++) {
        for (int c = center - 1; c <= center + 1; c++) {
            if (game_in_bounds(r, c)) {
                Piece piece = state->board[r][c].piece;
                if (piece != PIECE_NONE) {
                    int value = ai_is_king(piece) ? 20 : 10;
                    if (ai_piece_belongs_to_player(piece, player)) {
                        score += value;
                    } else {
                        score -= value;
                    }
                }
            }
        }
    }
    
    return score;
}

/**
 * \fn bool ai_is_endgame(const AI_GameState *state)
 * \brief Vérifier si on est en phase de fin de partie
 * \param state État de jeu
 * \return true si fin de partie, false sinon
 */
bool ai_is_endgame(const AI_GameState *state) {
    if (!state) return false;
    
    int total_pieces = state->blue_pieces + state->red_pieces;
    return total_pieces <= 6 || state->turns_remaining <= 10;
}

/**
 * \fn int ai_evaluate_position(AI_Context *ctx, const AI_GameState *state, AI_Player player)
 * \brief Évalue complètement une position
 * \param ctx Contexte IA
 * \param state État de jeu
 * \param player Joueur à évaluer
 * \return Score d'évaluation complet
 */
int ai_evaluate_position(AI_Context *ctx, const AI_GameState *state, AI_Player player) {
    if (!ctx || !state) return 0;
    
    // Vérification des conditions de victoire immédiate
    if (state->blue_kings == 0) {
        return (player == AI_PLAYER_RED) ? AI_INFINITY : -AI_INFINITY;
    }
    if (state->red_kings == 0) {
        return (player == AI_PLAYER_BLUE) ? AI_INFINITY : -AI_INFINITY;
    }
    
    int score = 0;
    
    // 1. Évaluation matérielle (40% du score)
    score += ai_evaluate_material(state, player) * 4;
    
    // 2. Position du roi (30% du score)
    score += ai_evaluate_king_position(state, player) * 3;
    
    // 3. Mobilité (20% du score)
    score += ai_evaluate_mobility(state, player) * 2;
    
    // 4. Contrôle du centre (10% du score)
    score += ai_evaluate_center_control(state, player);
    
    // Ajustements selon la phase de jeu
    if (ai_is_endgame(state)) {
        ctx->endgame_mode = true;
        // En fin de partie, la position du roi devient plus importante
        score += ai_evaluate_king_position(state, player) * 2;
    }
    
    return score;
}

// ============================================================================
// Recherche quiescente
// ============================================================================

/**
 * \fn int ai_quiescence_search(AI_Context *ctx, AI_GameState *state, int alpha, int beta, bool maximizing_player)
 * \brief Recherche quiescente pour évaluer les positions tactiquement stables
 * \param ctx Contexte IA
 * \param state État de jeu
 * \param alpha Valeur alpha pour l'élagage
 * \param beta Valeur beta pour l'élagage
 * \param maximizing_player true si le joueur maximise, false sinon
 * \return Score évalué de la position
 */
int ai_quiescence_search(AI_Context *ctx, AI_GameState *state, 
                        int alpha, int beta, bool maximizing_player) {
    if (!ctx || !state) return 0;
    
    ctx->nodes_searched++;
    
    AI_Player player = maximizing_player ? ctx->ai_player : ai_get_opponent(ctx->ai_player);
    int stand_pat = ai_evaluate_position(ctx, state, ctx->ai_player);
    
    if (maximizing_player) {
        if (stand_pat >= beta) return beta;
        if (alpha < stand_pat) alpha = stand_pat;
    } else {
        if (stand_pat <= alpha) return alpha;
        if (beta > stand_pat) beta = stand_pat;
    }
    
    // Génère seulement les coups tactiques
    AI_Move moves[AI_MAX_MOVES];
    int move_count;
    ai_generate_moves(state, player, moves, &move_count);
    
    // Filtre les coups tactiques
    AI_Move tactical_moves[AI_MAX_MOVES];
    int tactical_count = 0;
    for (int i = 0; i < move_count && tactical_count < AI_MAX_MOVES; i++) {
        if (ai_is_tactical_move(state, &moves[i])) {
            tactical_moves[tactical_count++] = moves[i];
        }
    }
    
    if (tactical_count == 0) return stand_pat;
    
    ai_order_moves(tactical_moves, tactical_count, state);
    
    for (int i = 0; i < tactical_count; i++) {
        AI_GameState backup = *state;
        if (!ai_apply_move(state, &tactical_moves[i])) {
            *state = backup;
            continue;
        }
        
        int score = ai_quiescence_search(ctx, state, alpha, beta, !maximizing_player);
        *state = backup;
        
        if (maximizing_player) {
            if (score >= beta) {
                ctx->alpha_beta_cutoffs++;
                return beta;
            }
            if (score > alpha) alpha = score;
        } else {
            if (score <= alpha) {
                ctx->alpha_beta_cutoffs++;
                return alpha;
            }
            if (score < beta) beta = score;
        }
    }
    
    return maximizing_player ? alpha : beta;
}

// ============================================================================
// Algorithme Minimax avec Alpha-Beta
// ============================================================================
/**
 * \fn int ai_minimax_alpha_beta(AI_Context *ctx, AI_GameState *state, int depth, int alpha, int beta, bool maximizing_player, AI_Move *best_move)
 * \brief Algorithme Minimax avec élagage Alpha-Beta
 * \param ctx Contexte IA
 * \param state État de jeu
 * \param depth Profondeur de recherche
 * \param alpha Valeur alpha pour l'élagage
 * \param beta Valeur beta pour l'élagage
 * \param maximizing_player true si le joueur maximise, false sinon
 * \param best_move Meilleur coup trouvé
 * \return Score du meilleur coup
 */
int ai_minimax_alpha_beta(AI_Context *ctx, AI_GameState *state, int depth, 
                         int alpha, int beta, bool maximizing_player, 
                         AI_Move *best_move) {
    if (!ctx || !state) return 0;
    
    ctx->nodes_searched++;
    
    // Calcul du hash pour la table de transposition
    unsigned long long hash = ai_zobrist_hash(ctx, state);
    AI_TranspositionEntry *tt_entry = ai_probe_transposition(ctx, hash);
    
    // Vérification de la table de transposition
    if (tt_entry && tt_entry->depth >= depth) {
        if (best_move) *best_move = tt_entry->best_move;
        
        if (tt_entry->flag == EXACT) {
            return tt_entry->score;
        } else if (tt_entry->flag == LOWER_BOUND && tt_entry->score >= beta) {
            return beta;
        } else if (tt_entry->flag == UPPER_BOUND && tt_entry->score <= alpha) {
            return alpha;
        }
    }
    
    // Conditions de fin de recherche
    if (depth == 0) {
        return ai_quiescence_search(ctx, state, alpha, beta, maximizing_player);
    }
    
    AI_Player current_player = maximizing_player ? ctx->ai_player : ai_get_opponent(ctx->ai_player);
    
    // Vérification des conditions de fin de partie
    if ((current_player == AI_PLAYER_BLUE && state->blue_kings == 0) ||
        (current_player == AI_PLAYER_RED && state->red_kings == 0)) {
        return maximizing_player ? -AI_INFINITY : AI_INFINITY;
    }
    
    // Génération des coups
    AI_Move moves[AI_MAX_MOVES];
    int move_count;
    ai_generate_moves(state, current_player, moves, &move_count);
    
    if (move_count == 0) {
        // Aucun coup possible - défaite
        return maximizing_player ? -AI_INFINITY : AI_INFINITY;
    }
    
    // Ordonnancement des coups
    ai_order_moves(moves, move_count, state);
    
    AI_Move local_best_move = {0};
    int best_score = maximizing_player ? -AI_INFINITY : AI_INFINITY;
    
    for (int i = 0; i < move_count; i++) {
        AI_GameState backup = *state;
        
        if (!ai_apply_move(state, &moves[i])) {
            *state = backup;
            continue;
        }
        
        AI_Move child_best_move;
        int score = ai_minimax_alpha_beta(ctx, state, depth - 1, alpha, beta, 
                                         !maximizing_player, &child_best_move);
        
        *state = backup; // Restauration de l'état
        
        if (maximizing_player) {
            if (score > best_score) {
                best_score = score;
                local_best_move = moves[i];
            }
            if (score >= beta) {
                ctx->alpha_beta_cutoffs++;
                break; // Coupure Beta
            }
            if (score > alpha) alpha = score;
        } else {
            if (score < best_score) {
                best_score = score;
                local_best_move = moves[i];
            }
            if (score <= alpha) {
                ctx->alpha_beta_cutoffs++;
                break; // Coupure Alpha
            }
            if (score < beta) beta = score;
        }
    }
    
    // Stockage en table de transposition
    int flag = EXACT;
    if (best_score <= alpha) flag = UPPER_BOUND;
    else if (best_score >= beta) flag = LOWER_BOUND;
    
    ai_store_transposition(ctx, hash, depth, best_score, &local_best_move, flag);
    
    if (best_move) *best_move = local_best_move;
    return best_score;
}

// ============================================================================
// Interface principale
// ============================================================================

/**
 * \fn bool ai_choose_best_move_from_board(const Square board[BOARD_SIZE][BOARD_SIZE], AI_Player player, AI_Move *best_move)
 * \brief Choisir le meilleur coup à partir d'un plateau donné
 * \param board Plateau de jeu
 * \param player Joueur pour lequel choisir le coup
 * \param best_move Meilleur coup trouvé
 * \return true si un coup a été trouvé, false sinon
 */
bool ai_choose_best_move_from_board(const Square board[BOARD_SIZE][BOARD_SIZE], 
                                   AI_Player player, AI_Move *best_move) {
    if (!board || !best_move) return false;
    
    // Initialisation du contexte si nécessaire
    if (!g_context_initialized) {
        ai_init_context(&g_ai_context, player);
        g_context_initialized = true;
    }
    
    // Conversion du plateau en état IA
    AI_GameState state = {0};
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            state.board[r][c] = board[r][c];
        }
    }
    state.current_player = player;
    
    // Mise à jour des statistiques
    ai_convert_game_state((GameState*)&state, &state); // Simplification
    
    // Réinitialisation des compteurs
    g_ai_context.nodes_searched = 0;
    g_ai_context.alpha_beta_cutoffs = 0;
    
    // Ajustement de la profondeur selon la phase de jeu
    int search_depth = AI_MAX_DEPTH; // Profondeur réduite pour des tests rapides
    if (ai_is_endgame(&state)) {
        search_depth = AI_MAX_DEPTH + 1; // Un peu plus profond en fin de partie
    }
    
    printf("IA commence la recherche (profondeur %d)...\n", search_depth);
    
    // Recherche du meilleur coup
    AI_Move found_move = {0};
    int score = ai_minimax_alpha_beta(&g_ai_context, &state, search_depth, 
                                     -AI_INFINITY, AI_INFINITY, true, &found_move);
    
    printf("IA: Noeuds explorés: %d, Coupures Alpha-Beta: %d, Score: %d\n", 
           g_ai_context.nodes_searched, g_ai_context.alpha_beta_cutoffs, score);
    
    // Vérification que le coup trouvé est valide
    if (found_move.r0 == 0 && found_move.c0 == 0 && 
        found_move.r1 == 0 && found_move.c1 == 0) {
        // Aucun coup trouvé par minimax, prendre le premier coup possible
        AI_Move moves[AI_MAX_MOVES];
        int move_count;
        ai_generate_moves(&state, player, moves, &move_count);
        
        if (move_count > 0) {
            found_move = moves[0];
            printf("IA: Utilisation du coup de secours\n");
        } else {
            printf("IA: Aucun coup possible!\n");
            return false;
        }
    }
    
    *best_move = found_move;
    
    char from[8], to[8];
    game_coord_to_str(found_move.r0, found_move.c0, from, sizeof from);
    game_coord_to_str(found_move.r1, found_move.c1, to, sizeof to);
    
    printf("IA choisit: %s -> %s", from, to);
    if (found_move.is_seultou) printf(" (SEULTOU)");
    if (found_move.is_linca) printf(" (LINCA)");
    if (found_move.captures_king) printf(" (CAPTURE ROI)");
    if (found_move.reaches_city) printf(" (VICTOIRE)");
    printf("\n");
    
    return true;
}