#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @file game.h
 * @brief Définitions des types et structures pour le jeu Krojanty.
 * @author kylian.adam@uha.fr
 * @note Fichier documenté par hadil.abahmane@uha.fr
 */

#define BOARD_SIZE 9
#ifdef TEST
#define TURN_LIMIT_TOTAL 6
#else
#define TURN_LIMIT_TOTAL 64
#endif

/**
 * \enum CellKind
 * \brief Enumération représentant le type de cellule sur le plateau
 */
typedef enum { /**Case vide du plateau*/
    CELL_EMPTY,      /** Cité du joueur Bleu (coin du plateau)*/
    CELL_CITY_BLUE,  /** Cité du joueur Rouge (coin du plateau)*/
    CELL_CITY_RED    
} CellKind;

/**
 * \enum Piece
 * \brief Enumération représentant les différentes pièces du jeu
 */
typedef enum {/** Aucune pièce sur la case */
    PIECE_NONE,         /** Soldat du joueur Bleu */ 
    PIECE_SOLDIER_BLUE,  /** Roi du joueur Bleu */
    PIECE_KING_BLUE,     /** Soldat du joueur Rouge */
    PIECE_SOLDIER_RED,   /** Roi du joueur Rouge */
    PIECE_KING_RED       
} Piece;

/**
 * \enum Player
 * \brief Enumération représentant les joueurs
 */
typedef enum {
    PLAYER_NONE,
    PLAYER_BLUE,
    PLAYER_RED
} Player;

/**
 * \enum Control
 * \brief Enumération représentant le contrôle d'une case
 */
typedef enum {
    CTRL_NONE,
    CTRL_BLUE,
    CTRL_RED
} Control;

/**
 * \enum GameMode
 * \brief Enumération représentant les modes de jeu
 */
typedef enum {
    MODE_LOCAL,
    MODE_SERVER,
    MODE_CLIENT
} GameMode;

/**
 * \struct Square
 * \brief Structure représentant une case du plateau de jeu
 * 
 * Une case contient à la fois une information sur son type (CellKind)
 * et la pièce qui s'y trouve éventuellement (Piece).
 */
typedef struct {/**Type de la cellule (vide, cité bleue, cité rouge) */
    CellKind cell;  /** Pièce présente sur la cellule (s'il y en a une) */
    Piece    piece; 
} Square;

/**
 * \struct GameState
 * \brief Structure représentant l'état complet du jeu
 */
typedef struct {
    Square board[BOARD_SIZE][BOARD_SIZE];
    Control ctrl[BOARD_SIZE][BOARD_SIZE];
    Player current_player;
    bool pending_victory;
    Player pending_winner;
    char pending_reason[256];
    int blue_score;
    int red_score;
    int selection_r;
    int selection_c;
    bool has_selection;
    
    // Champs additionnels utilisés dans le code
    Player turn;
    int turns_left;
    int score_blue;
    int score_red;
    bool game_over;
    bool have_selection;
    int sel_r;
    int sel_c;
    GameMode game_mode;
} GameState;

// Déclarations des fonctions
bool game_in_bounds(int r, int c);
Player game_piece_owner(Piece piece);
const char *game_player_name(Player p);
void game_coord_to_str(int r, int c, char *out, size_t out_sz);
bool game_str_to_coord(const char *str, int *r, int *c);
void game_init(GameState *game, GameMode mode);
void game_reset(GameState *game);
bool game_can_move(const GameState *game, int r0, int c0, int r1, int c1);
bool game_can_seultou(const GameState *game, int r0, int c0, int r1, int c1, int er, int ec);
bool game_apply_seultou(GameState *game, int r0, int c0, int r1, int c1, int er, int ec);
void game_update_scores(GameState *game);
void game_get_legal_moves(const GameState *game, int r, int c, int moves[][2], int *count);
int play_local(bool ia_enabled);
int play_client(bool ia_enabled, char *ip, unsigned short port);
int play_server(bool ia_enabled, unsigned short port);
int is_number(const char *str);
int validate_ip_port(const char *ip_port);

#endif