/**
 * @file ui.h
 * @brief Déclarations de l'Interface utilisateur du jeu utilisant GTK.
 * @author sofiane.bouguerri@uha.fr  
 * @note Documenté par becker.obie-obolo@uha.fr
 * 
 * @note Ce fichier définit les structures et fonctions permettant de gérer
 * l'affichage graphique du jeu, les interactions avec l'utilisateur
 * et la communication avec le moteur du jeu.
 *
 * L'interface utilisateur est responsable de :
 * - Afficher le plateau de jeu et les informations associées (scores, statut, messages de victoire ou de match nul),
 * - Réagir aux actions de l'utilisateur, comme cliquer sur une cellule ou demander de rejouer,
 * - Notifier le moteur du jeu de ces actions via des callbacks,
 * - Mettre à jour dynamiquement l'affichage en fonction de l'état actuel du jeu.
 *
 * L'interface utilisateur n'applique pas les règles du jeu elle-même ; 
 * elle se contente de représenter visuellement l'état du jeu et de transmettre
 * les actions de l'utilisateur au moteur de jeu.
 *  
 * @note Les fonctions sans sortie spécifiée ne retournent aucune valeur (type void).
 
 */

#ifndef UI_H
#define UI_H

#include <gtk/gtk.h>
#include "game.h"

/**
 * @brief État opaque de l'UI.
 *
 * Cette structure contient toutes les données internes
 * nécessaires à la gestion de l'interface graphique.
 */
typedef struct UIState UIState;

/**
 * @brief Callbacks pour communication avec le moteur de jeu.
 *
 * Cette structure permet à l'UI de notifier le moteur
 * de jeu lorsque certaines actions utilisateur se produisent.
 */
typedef struct {
    /**
     * @brief Callback déclenché lorsqu'une cellule est cliquée.
     * @param game_data Pointeur vers les données du jeu.
     * @param r Ligne de la cellule cliquée.
     * @param c Colonne de la cellule cliquée.
     * @return true si le clic est valide et traité, false sinon.
     */
    bool (*on_cell_clicked)(void *game_data, int r, int c);

    /**
     * @brief Callback déclenché lorsqu'un joueur demande à rejouer.
     * @param game_data Pointeur vers les données du jeu.
     */
    void (*on_replay_requested)(void *game_data);

    /** @brief Pointeur vers les données du jeu. */
    void *game_data;
} UICallbacks;

/**
 * @fn UIState* ui_create(GameMode mode, bool ia_enabled)
 * @brief Crée l'interface utilisateur.
 * @param mode Mode de jeu (ex: joueur contre joueur ou IA).
 * @param ia_enabled Indique si l'IA est activée.
 * @return Pointeur vers l'état UI créé.
 */
UIState* ui_create(GameMode mode, bool ia_enabled);

/**
 * @fn ui_destroy(UIState *ui)
 * @brief Détruit l'interface utilisateur et libère la mémoire associée.
 * @param ui Pointeur vers l'état UI à détruire.
 */
void ui_destroy(UIState *ui);

/**
 * @fn ui_set_callbacks(UIState *ui, UICallbacks callbacks)
 * @brief Définit les callbacks pour l'UI.
 * @param ui Pointeur vers l'état UI.
 * @param callbacks Structure contenant les callbacks à utiliser.
 */
void ui_set_callbacks(UIState *ui, UICallbacks callbacks);

/**
 * @fn GtkWindow* ui_get_window(UIState *ui)
 * @brief Récupère la fenêtre principale GTK.
 * @param ui Pointeur vers l'état UI.
 * @return Pointeur vers la GtkWindow principale.
 */
GtkWindow* ui_get_window(UIState *ui);

/**
 * @fn ui_show(UIState *ui)
 * @brief Affiche la fenêtre principale.
 * @param ui Pointeur vers l'état UI.
 */
void ui_show(UIState *ui);

/**
 * @fn ui_load_css(void)
 * @brief Charge le fichier CSS pour styliser l'interface.
 */
void ui_load_css(void);

/**
 * @fn ui_refresh_board(UIState *ui, const GameState *game)
 * @brief Met à jour l'affichage du plateau de jeu.
 * @param ui Pointeur vers l'état UI.
 * @param game État actuel du jeu.
 */
void ui_refresh_board(UIState *ui, const GameState *game);

/**
 * @fn ui_refresh_scores(UIState *ui, int blue, int red)
 * @brief Met à jour l'affichage des scores.
 * @param ui Pointeur vers l'état UI.
 * @param blue Score du joueur bleu.
 * @param red Score du joueur rouge.
 */
void ui_refresh_scores(UIState *ui, int blue, int red);

/**
 * @fn ui_refresh_panel(UIState *ui, const GameState *game)
 * @brief Met à jour le panneau latéral de l'UI.
 * @param ui Pointeur vers l'état UI.
 * @param game État actuel du jeu.
 */
void ui_refresh_panel(UIState *ui, const GameState *game);

/**
 * @fn ui_set_status(UIState *ui, const char *text)
 * @brief Met à jour le texte de statut.
 * @param ui Pointeur vers l'état UI.
 * @param text Texte à afficher.
 */
void ui_set_status(UIState *ui, const char *text);

/**
 * @fn ui_show_selection(UIState *ui, int r, int c)
 * @brief Affiche une sélection sur une cellule.
 * @param ui Pointeur vers l'état UI.
 * @param r Ligne de la cellule.
 * @param c Colonne de la cellule.
 */
void ui_show_selection(UIState *ui, int r, int c);

/**
 * @fn ui_clear_selection(UIState *ui)
 * @brief Supprime toute sélection affichée.
 * @param ui Pointeur vers l'état UI.
 */
void ui_clear_selection(UIState *ui);

/**
 * @fn ui_show_move_hints(UIState *ui, const GameState *game, int r, int c)
 * @brief Affiche les coups possibles depuis une cellule.
 * @param ui Pointeur vers l'état UI.
 * @param game État actuel du jeu.
 * @param r Ligne de la cellule.
 * @param c Colonne de la cellule.
 */
void ui_show_move_hints(UIState *ui, const GameState *game, int r, int c);

/**
 * @fn ui_clear_move_hints(UIState *ui)
 * @brief Supprime les coups possibles affichés.
 * @param ui Pointeur vers l'état UI.
 */
void ui_clear_move_hints(UIState *ui);

/**
 * @fn ui_show_victory(UIState *ui, Player winner, const char *reason)
 * @brief Affiche la victoire d'un joueur.
 * @param ui Pointeur vers l'état UI.
 * @param winner Joueur gagnant.
 * @param reason Raison de la victoire.
 */
void ui_show_victory(UIState *ui, Player winner, const char *reason);

/**
 * @fn ui_show_draw(UIState *ui, const char *reason)
 * @brief Affiche une partie nulle.
 * @param ui Pointeur vers l'état UI.
 * @param reason Raison du match nul.
 */
void ui_show_draw(UIState *ui, const char *reason);

/**
 * @fn ui_hide_victory_overlay(UIState *ui)
 * @brief Cache l'overlay de victoire ou de match nul.
 * @param ui Pointeur vers l'état UI.
 */
void ui_hide_victory_overlay(UIState *ui);

/**
 * @fn ui_set_board_sensitive(UIState *ui, bool sensitive)
 * @brief Active ou désactive la sensibilité du plateau.
 * @param ui Pointeur vers l'état UI.
 * @param sensitive true pour activer, false pour désactiver.
 */
void ui_set_board_sensitive(UIState *ui, bool sensitive);

/**
 * @fn ui_set_replay_status(UIState *ui, int ready_count)
 * @brief Met à jour le statut des demandes de replay.
 * @param ui Pointeur vers l'état UI.
 * @param ready_count Nombre de joueurs prêts à rejouer.
 */
void ui_set_replay_status(UIState *ui, int ready_count);

#endif // UI_H
