/**
 * @file ui.c
 * @author sofiane.bouguerri@uha.fr  
 * @note Documenté par becker.obie-obolo@uha.fr
 * @brief Implémentation de l'Interface utilisateur du jeu utilisant GTK.
 * 
 * @note Ce fichier coordonne les structures et fonctions permettant de faire fonctionner
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


#include "ui.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// -----------------------------------------------------------------------------
// Structures internes de l'interface graphique
// -----------------------------------------------------------------------------

/**
 * @struct Confetti
 * @brief Représente une particule de confetti utilisée dans l'animation de victoire.
 *
 * Chaque confetti possède :
 * - une position (x, y),
 * - une vitesse (vx, vy),
 * - une rotation et une vitesse de rotation,
 * - une taille,
 * - une couleur (RGBA).
 */
typedef struct {
    float x;       /**< Position horizontale */
    float y;       /**< Position verticale */
    float vx;      /**< Vitesse horizontale */
    float vy;      /**< Vitesse verticale */
    float rot;     /**< Rotation courante en radians */
    float vrot;    /**< Vitesse de rotation */
    float size;    /**< Taille du confetti */
    GdkRGBA color; /**< Couleur (rouge, vert, bleu, alpha) */
} Confetti;

/**
 * @struct ConfettiState
 * @brief État global des confettis pour l'animation de victoire.
 *
 * Contient :
 * - un tableau dynamique de particules,
 * - l'identifiant du tick GTK qui anime les confettis.
 */
typedef struct {
    GArray *particles; /**< Tableau dynamique de particules Confetti */
    guint   tick_id;   /**< Identifiant du callback d’animation (g_timeout_add / tick) */
} ConfettiState;

/**
 * @struct UIState
 * @brief État principal de l'interface utilisateur.
 *
 * Cette structure regroupe tous les widgets GTK nécessaires pour gérer :
 * - la fenêtre et le plateau de jeu,
 * - les panneaux d’informations et de scores,
 * - l’overlay de victoire et l’animation de confettis,
 * - les sélections et indices de coups possibles,
 * - la communication avec le moteur de jeu via les callbacks.
 */
struct UIState {
    GtkApplication *app;              /**< Application GTK (facultatif, utilisé si intégré à GtkApplication) */
    GtkWindow      *win;              /**< Fenêtre principale */
    GtkGrid        *grid;             /**< Grille représentant le plateau */
    GtkWidget      *cells[BOARD_SIZE][BOARD_SIZE]; /**< Boutons correspondant aux cases du plateau */

    GtkOverlay     *overlay;          /**< Overlay global pour superposer des éléments (par ex. victoire) */
    GtkWidget      *victory_layer;    /**< Calque affiché à la fin de la partie */
    GtkWidget      *victory_label;    /**< Label du message de victoire */
    GtkWidget      *defeat_label;     /**< Label du message de défaite */
    GtkWidget      *confetti_area;    /**< Zone de dessin pour les confettis */
    GtkWidget      *replay_btn;       /**< Bouton pour relancer une partie */
    GtkWidget      *replay_status_overlay; /**< Overlay indiquant l'état des demandes de rejouer */

    GtkWidget      *sidebar;          /**< Panneau latéral regroupant infos et scores */
    GtkWidget      *panel_role_lbl;   /**< Label indiquant le rôle du joueur (ex : "Bleu" ou "Rouge") */
    GtkWidget      *panel_turn_lbl;   /**< Label affichant le joueur dont c’est le tour */
    GtkWidget      *panel_turn_hint;  /**< Indication supplémentaire pour le joueur courant */
    GtkWidget      *panel_turns_lbl;  /**< Label indiquant le nombre de tours restants */
    GtkWidget      *panel_replay_status; /**< Label affichant l'état du bouton "Rejouer" */

    GtkWidget      *score_blue_lbl;   /**< Label affichant le score du joueur bleu */
    GtkWidget      *score_red_lbl;    /**< Label affichant le score du joueur rouge */
    GtkWidget      *status_lbl;       /**< Label de statut général (messages divers) */

    bool            have_selection;   /**< Indique si une case est actuellement sélectionnée */
    int             sel_r;            /**< Ligne de la case sélectionnée */
    int             sel_c;            /**< Colonne de la case sélectionnée */

    guint           hint_timer_id;    /**< Identifiant du timer pour afficher les indices de coups possibles */
    GPtrArray      *hint_widgets;     /**< Tableau de widgets correspondant aux indices affichés */
    int             hint_index;       /**< Position courante dans la séquence d’indices */

    GameMode        game_mode;        /**< Mode de jeu (ex. joueur vs joueur, joueur vs IA) */
    bool            ia_enabled;       /**< Indique si l’IA est activée */

    UICallbacks     callbacks;        /**< Callbacks permettant la communication avec le moteur de jeu */
    ConfettiState   confetti;         /**< État de l'animation de confettis */
};

// -----------------------------------------------------------------------------
// Helpers UI communs
// -----------------------------------------------------------------------------


/**
 * @fn GtkWidget *make_coord_label(const char *txt)
 * @brief Crée un label utilisé pour afficher les coordonnées autour du plateau.
 *
 * Le label généré est centré, expansible horizontalement, et possède
 * une classe CSS spéciale ("coord") permettant de le styliser facilement.
 *
 * @param txt Texte à afficher dans le label.
 * @return GtkWidget* Un pointeur vers le widget label configuré.
 */

static GtkWidget *make_coord_label(const char *txt) {
    GtkWidget *l = gtk_label_new(txt);
    gtk_widget_add_css_class(l, "coord");
    gtk_widget_set_hexpand(l, TRUE);
    gtk_widget_set_halign(l, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(l, GTK_ALIGN_CENTER);
    gtk_widget_set_size_request(l, 44, -1);
    return l;
}

/**
 * @fn on_cell_clicked_internal(GtkButton *btn, gpointer user_data)
 * @brief Callback interne appelé lorsqu'une cellule du plateau est cliquée.
 *
 * Cette fonction récupère les coordonnées (ligne, colonne) associées au bouton cliqué
 * puis appelle le callback `on_cell_clicked` défini dans @ref UICallbacks si disponible.
 *
 * @param btn Le bouton GTK qui a été cliqué.
 * @param user_data Pointeur vers l'instance de @ref UIState associée.
 */

static void on_cell_clicked_internal(GtkButton *btn, gpointer user_data) {
    UIState *ui = (UIState*)user_data;
    int r = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(btn), "r"));
    int c = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(btn), "c"));
    if (ui->callbacks.on_cell_clicked)
        ui->callbacks.on_cell_clicked(ui->callbacks.game_data, r, c);
}

/**
 * @brief Callback interne appelé lorsqu'un joueur clique sur le bouton "Rejouer".
 * @fn on_replay_clicked_internal(GtkButton *btn, gpointer user_data)
 *
 * Appelle la fonction `on_replay_requested` définie dans @ref UICallbacks
 * pour notifier le moteur du jeu qu’un nouveau match est demandé.
 *
 * @param btn Le bouton GTK "Rejouer" (non utilisé).
 * @param user_data Pointeur vers l'instance de @ref UIState associée.
 */

static void on_replay_clicked_internal(GtkButton *btn, gpointer user_data) {
    (void)btn;
    UIState *ui = (UIState*)user_data;
    if (ui->callbacks.on_replay_requested)
        ui->callbacks.on_replay_requested(ui->callbacks.game_data);
}

// -----------------------------------------------------------------------------
// Confettis
// -----------------------------------------------------------------------------


/**
 * @brief Fonction de mise à jour des confettis (appelée à chaque tick GTK).
 * @fn gboolean confetti_tick(GtkWidget *widget, GdkFrameClock *clock, gpointer user_data)
 *
 * Déplace chaque particule de confetti en fonction de sa vitesse et de la gravité,
 * applique des rebonds sur les bords de la fenêtre, et recycle les confettis
 * qui sortent de l’écran pour maintenir un flux continu.
 *
 * @param widget Zone de dessin des confettis.
 * @param clock Horloge GTK utilisée pour synchroniser les frames.
 * @param user_data Pointeur vers l'instance de @ref UIState associée.
 * @return gboolean Retourne G_SOURCE_CONTINUE pour maintenir l’animation,
 *         ou G_SOURCE_REMOVE pour l’arrêter si plus de particules.
 */

static gboolean confetti_tick(GtkWidget *widget, GdkFrameClock *clock, gpointer user_data) {
    (void)widget;
    (void)clock;
    UIState *ui = (UIState*)user_data;
    ConfettiState *st = &ui->confetti;
    if (!st->particles) return G_SOURCE_REMOVE;

    double dt = 1.0 / 60.0;
    int w = gtk_widget_get_width(ui->confetti_area);
    int h = gtk_widget_get_height(ui->confetti_area);

    for (guint i = 0; i < st->particles->len; ++i) {
        Confetti *cf = &g_array_index(st->particles, Confetti, i);
        cf->x += cf->vx * dt;
        cf->y += cf->vy * dt;
        cf->vy += 220 * dt;
        cf->rot += cf->vrot * dt;

        if (cf->x < 0) { cf->x = 0; cf->vx *= -0.6f; }
        if (cf->x > w) { cf->x = w; cf->vx *= -0.6f; }
        if (cf->y > h + 20) {
            cf->x = g_random_double_range(0, w);
            cf->y = g_random_double_range(-h * 0.3, 0);
            cf->vx = g_random_double_range(-60, 60);
            cf->vy = g_random_double_range(120, 240);
            cf->rot = g_random_double_range(0, G_PI * 2);
        }
    }

    gtk_widget_queue_draw(ui->confetti_area);
    return G_SOURCE_CONTINUE;
}


/**
 * @brief Fonction de rendu des confettis.
 * @fn confetti_on_draw(GtkDrawingArea *area, cairo_t *cr, int w, int h, gpointer user_data)
 *
 * Dessine chaque particule de confetti sous forme de petit carré coloré
 * en utilisant Cairo. La position, rotation et taille de chaque confetti
 * sont prises en compte.
 *
 * @param area Zone de dessin GTK.
 * @param cr Contexte de dessin Cairo.
 * @param w Largeur disponible.
 * @param h Hauteur disponible.
 * @param user_data Pointeur vers l'instance de @ref UIState associée.
 */

static void confetti_on_draw(GtkDrawingArea *area, cairo_t *cr, int w, int h, gpointer user_data) {
    (void)area;
    (void)w;
    (void)h;
    UIState *ui = (UIState*)user_data;
    ConfettiState *st = &ui->confetti;
    if (!st->particles) return;

    cairo_save(cr);
    for (guint i = 0; i < st->particles->len; ++i) {
        Confetti *cf = &g_array_index(st->particles, Confetti, i);
        gdk_cairo_set_source_rgba(cr, &cf->color);
        cairo_translate(cr, cf->x, cf->y);
        cairo_rotate(cr, cf->rot);
        cairo_rectangle(cr, -cf->size * 0.5f, -cf->size * 0.5f, cf->size, cf->size);
        cairo_fill(cr);
        cairo_identity_matrix(cr);
    }
    cairo_restore(cr);
}


/**
 * @brief Initialise et démarre une nouvelle animation de confettis.
 * @fn start_confetti(UIState *ui)
 *
 * Crée un ensemble de particules aléatoires avec position, vitesse, rotation,
 * taille et couleur différentes. Ajoute ensuite un callback GTK (tick) pour
 * animer les confettis à 60 FPS.
 *
 * @param ui Pointeur vers l'instance de @ref UIState associée.
 */

static void start_confetti(UIState *ui) {
    int w = gtk_widget_get_width(GTK_WIDGET(ui->win));
    int h = gtk_widget_get_height(GTK_WIDGET(ui->win));
    if (w <= 0 || h <= 0) { w = 800; h = 540; }

    ConfettiState *st = &ui->confetti;
    if (!st->particles)
        st->particles = g_array_new(FALSE, FALSE, sizeof(Confetti));
    g_array_set_size(st->particles, 0);

    for (int i = 0; i < 220; ++i) {
        Confetti cf;
        cf.x = g_random_double_range(0, w);
        cf.y = g_random_double_range(-h * 0.5, 0);
        cf.vx = g_random_double_range(-60, 60);
        cf.vy = g_random_double_range(120, 260);
        cf.rot = g_random_double_range(0, G_PI * 2);
        cf.vrot = g_random_double_range(-4, 4);
        cf.size = g_random_double_range(6, 12);
        cf.color.red   = g_random_double_range(0.2, 1.0);
        cf.color.green = g_random_double_range(0.2, 1.0);
        cf.color.blue  = g_random_double_range(0.2, 1.0);
        cf.color.alpha = 1.0;
        g_array_append_val(st->particles, cf);
    }

    if (st->tick_id) {
        gtk_widget_remove_tick_callback(ui->confetti_area, st->tick_id);
        st->tick_id = 0;
    }

    if (ui->confetti_area && GTK_IS_WIDGET(ui->confetti_area))
        gtk_widget_set_visible(ui->confetti_area, TRUE);
    st->tick_id = gtk_widget_add_tick_callback(ui->confetti_area, confetti_tick, ui, NULL);
    gtk_widget_queue_draw(ui->confetti_area);
}

/**
 * @brief Arrête et nettoie l’animation des confettis.
 * @fn stop_confetti(UIState *ui) 
 *
 * Supprime le callback GTK responsable de l’animation,
 * libère la mémoire associée aux particules et cache la zone
 * de dessin des confettis.
 *
 * @param ui Pointeur vers l’instance de @ref UIState associée.
 */


static void stop_confetti(UIState *ui) {
    ConfettiState *st = &ui->confetti;
    if (st->tick_id && ui->confetti_area && GTK_IS_WIDGET(ui->confetti_area)) {
        gtk_widget_remove_tick_callback(ui->confetti_area, st->tick_id);
        st->tick_id = 0;
    }
    if (st->particles) {
        g_array_free(st->particles, TRUE);
        st->particles = NULL;
    }
    if (ui->confetti_area && GTK_IS_WIDGET(ui->confetti_area))
        gtk_widget_set_visible(ui->confetti_area, FALSE);
}

// -----------------------------------------------------------------------------
// Construction UI
// -----------------------------------------------------------------------------

/**
 * @brief Construit l’overlay de victoire affiché à la fin d’une partie.
 * @fn build_victory_overlay(UIState *ui)
 *
 * Cet overlay contient :
 * - une zone de dessin pour les confettis,
 * - un message de victoire (`victory_label`),
 * - un message de défaite optionnel (`defeat_label`),
 * - un bouton "Rejouer" pour redémarrer la partie,
 * - un label indiquant l’état de la demande de rejouer (en mode multijoueur).
 *
 * Les widgets sont configurés avec des classes CSS spécifiques
 * pour permettre un style personnalisé.
 *
 * @param ui Pointeur vers l’instance de @ref UIState associée.
 */

static void build_victory_overlay(UIState *ui) {
    GtkWidget *layer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_add_css_class(layer, "victory-overlay");
    gtk_widget_set_hexpand(layer, TRUE);
    gtk_widget_set_vexpand(layer, TRUE);
    gtk_widget_set_visible(layer, FALSE);
    ui->victory_layer = layer;

    GtkWidget *confetti = gtk_drawing_area_new();
    gtk_widget_set_hexpand(confetti, TRUE);
    gtk_widget_set_vexpand(confetti, TRUE);
    gtk_widget_set_visible(confetti, FALSE);
    gtk_widget_set_can_target(confetti, FALSE);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(confetti), confetti_on_draw, ui, NULL);
    ui->confetti_area = confetti;

    GtkWidget *inner = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_add_css_class(inner, "victory-banner");
    gtk_widget_set_halign(inner, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(inner, GTK_ALIGN_CENTER);

    GtkWidget *winner_lbl = gtk_label_new("Joueur Bleu a gagné !");
    gtk_widget_add_css_class(winner_lbl, "victory-text");
    ui->victory_label = winner_lbl;

    GtkWidget *defeat_lbl = gtk_label_new("");
    gtk_widget_add_css_class(defeat_lbl, "defeat-text");
    gtk_widget_set_visible(defeat_lbl, FALSE);
    ui->defeat_label = defeat_lbl;

    GtkWidget *btn = gtk_button_new_with_label("Rejouer");
    gtk_widget_add_css_class(btn, "replay-button");
    g_signal_connect(btn, "clicked", G_CALLBACK(on_replay_clicked_internal), ui);
    ui->replay_btn = btn;

    GtkWidget *replay_status = gtk_label_new("");
    gtk_widget_add_css_class(replay_status, "replay-status");
    ui->replay_status_overlay = replay_status;
    if (ui->game_mode == MODE_LOCAL)
        gtk_widget_set_visible(replay_status, FALSE);

    gtk_box_append(GTK_BOX(inner), winner_lbl);
    gtk_box_append(GTK_BOX(inner), defeat_lbl);
    gtk_box_append(GTK_BOX(inner), btn);
    gtk_box_append(GTK_BOX(inner), replay_status);

    gtk_overlay_add_overlay(ui->overlay, confetti);
    gtk_overlay_add_overlay(ui->overlay, layer);
    gtk_box_append(GTK_BOX(layer), inner);
}

/**
 * @brief Construit la barre latérale d’informations (sidebar).
 * @fn build_sidebar(UIState *ui)
 *
 * Cette sidebar regroupe plusieurs sections :
 * - **Profil** : rôle du joueur (ex. bleu ou rouge),
 * - **Partie** : informations sur le tour en cours et le nombre de tours restants,
 * - **Scores** : scores actuels des joueurs bleu et rouge,
 * - **Statut** : messages et indications de jeu.
 *
 * Les éléments sont regroupés dans des cadres (`GtkFrame`) pour plus de clarté
 * et sont stylisés via des classes CSS.
 *
 * @param ui Pointeur vers l’instance de @ref UIState associée.
 */

static void build_sidebar(UIState *ui) {
    GtkWidget *sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_add_css_class(sidebar, "info-panel");
    gtk_widget_set_margin_top(sidebar, 8);
    gtk_widget_set_margin_bottom(sidebar, 8);
    gtk_widget_set_margin_end(sidebar, 8);
    gtk_widget_set_size_request(sidebar, 240, -1);
    ui->sidebar = sidebar;

    GtkWidget *f_profile = gtk_frame_new("Profil");
    gtk_widget_add_css_class(f_profile, "panel-frame");
    GtkWidget *box_profile = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    GtkWidget *role = gtk_label_new("");
    gtk_widget_add_css_class(role, "badge");
    gtk_box_append(GTK_BOX(box_profile), role);
    gtk_frame_set_child(GTK_FRAME(f_profile), box_profile);
    ui->panel_role_lbl = role;

    GtkWidget *f_game = gtk_frame_new("Partie");
    gtk_widget_add_css_class(f_game, "panel-frame");
    GtkWidget *box_game = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    GtkWidget *turn_lbl = gtk_label_new("");
    GtkWidget *turns_left_lbl = gtk_label_new("");
    gtk_widget_add_css_class(turns_left_lbl, "counter");
    gtk_box_append(GTK_BOX(box_game), turn_lbl);
    gtk_box_append(GTK_BOX(box_game), turns_left_lbl);
    gtk_frame_set_child(GTK_FRAME(f_game), box_game);
    ui->panel_turn_lbl = turn_lbl;
    ui->panel_turns_lbl = turns_left_lbl;

    GtkWidget *f_scores = gtk_frame_new("Scores");
    gtk_widget_add_css_class(f_scores, "panel-frame");
    GtkWidget *box_scores = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    GtkWidget *lbl_blue = gtk_label_new("Bleu : 0");
    GtkWidget *lbl_red  = gtk_label_new("Rouge : 0");
    gtk_widget_add_css_class(lbl_blue, "score-blue");
    gtk_widget_add_css_class(lbl_red,  "score-red");
    gtk_box_append(GTK_BOX(box_scores), lbl_blue);
    gtk_box_append(GTK_BOX(box_scores), lbl_red);
    gtk_frame_set_child(GTK_FRAME(f_scores), box_scores);
    ui->score_blue_lbl = lbl_blue;
    ui->score_red_lbl  = lbl_red;

    GtkWidget *f_status = gtk_frame_new("Statut");
    gtk_widget_add_css_class(f_status, "panel-frame");
    GtkWidget *box_status = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    GtkWidget *turn_hint = gtk_label_new("");
    GtkWidget *status_lbl = gtk_label_new("");
    gtk_widget_add_css_class(turn_hint, "turn-status");
    gtk_widget_add_css_class(status_lbl, "status-text");
    gtk_box_append(GTK_BOX(box_status), turn_hint);
    gtk_box_append(GTK_BOX(box_status), status_lbl);
    gtk_frame_set_child(GTK_FRAME(f_status), box_status);
    ui->panel_turn_hint = turn_hint;
    ui->status_lbl = status_lbl;

    gtk_box_append(GTK_BOX(sidebar), f_profile);
    gtk_box_append(GTK_BOX(sidebar), f_game);
    gtk_box_append(GTK_BOX(sidebar), f_scores);
    gtk_box_append(GTK_BOX(sidebar), f_status);
}


/**
 * @brief Construit l’interface graphique principale du jeu.
 * @fn build_ui_internal(UIState *ui)
 *
 * Cette fonction initialise la fenêtre principale, configure son titre
 * en fonction du mode de jeu (local, serveur ou client), et crée tous les
 * éléments de l’interface :
 * - La barre d’en-tête avec le titre.
 * - La zone racine horizontale.
 * - Un GtkOverlay contenant :
 *   - La grille principale avec les coordonnées (lignes et colonnes).
 *   - Le plateau de jeu composé de boutons (cases).
 * - La superposition de victoire (overlay) avec animations de confettis,
 *   message de victoire/défaite et bouton de rejouer.
 * - La barre latérale d’informations (profil, scores, tours, statut).
 *
 * @param ui Pointeur vers la structure d’état de l’interface utilisateur.
 */

static void build_ui_internal(UIState *ui) {
    if (ui->win) return;

    GtkWindow *win = GTK_WINDOW(gtk_window_new());
    gtk_window_set_title(win, "Krojanty — IHM (GTK4)");
    gtk_window_set_default_size(win, 800, 500);
    gtk_window_set_resizable(win, FALSE);
    ui->win = win;

    GtkWidget *header = gtk_header_bar_new();
    char title_txt[128];
    if (ui->game_mode == MODE_SERVER)
        strcpy(title_txt, "Krojanty — Mode Réseau (Serveur)");
    else if (ui->game_mode == MODE_CLIENT)
        strcpy(title_txt, "Krojanty — Mode Réseau (Client)");
    else
        strcpy(title_txt, "Krojanty — Mode Local");
    GtkWidget *title = gtk_label_new(title_txt);
    gtk_header_bar_set_title_widget(GTK_HEADER_BAR(header), title);
    gtk_window_set_titlebar(win, header);

    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_window_set_child(win, root);

    GtkWidget *overlay = gtk_overlay_new();
    ui->overlay = GTK_OVERLAY(overlay);
    gtk_box_append(GTK_BOX(root), overlay);

    GtkWidget *frame = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(frame), 4);
    gtk_grid_set_column_spacing(GTK_GRID(frame), 4);
    gtk_widget_set_margin_top(frame, 8);
    gtk_widget_set_margin_bottom(frame, 8);
    gtk_widget_set_margin_start(frame, 8);
    gtk_widget_set_margin_end(frame, 8);
    gtk_overlay_set_child(ui->overlay, frame);

    GtkWidget *top_letters = gtk_grid_new();
    GtkWidget *bot_letters = gtk_grid_new();
    gtk_grid_set_column_homogeneous(GTK_GRID(top_letters), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(bot_letters), TRUE);
    for (int c = 0; c < BOARD_SIZE; ++c) {
        char buf[2] = { (char)('A' + c), 0 };
        gtk_grid_attach(GTK_GRID(top_letters), make_coord_label(buf), c, 0, 1, 1);
        gtk_grid_attach(GTK_GRID(bot_letters), make_coord_label(buf), c, 0, 1, 1);
    }

    GtkWidget *left_nums = gtk_grid_new();
    GtkWidget *right_nums = gtk_grid_new();
    gtk_grid_set_row_homogeneous(GTK_GRID(left_nums), TRUE);
    gtk_grid_set_row_homogeneous(GTK_GRID(right_nums), TRUE);
    for (int r = 0; r < BOARD_SIZE; ++r) {
        char buf[3];
        snprintf(buf, sizeof buf, "%d", BOARD_SIZE - r);
        gtk_grid_attach(GTK_GRID(left_nums),  make_coord_label(buf), 0, r, 1, 1);
        gtk_grid_attach(GTK_GRID(right_nums), make_coord_label(buf), 0, r, 1, 1);
    }

    ui->grid = GTK_GRID(gtk_grid_new());
    gtk_widget_add_css_class(GTK_WIDGET(ui->grid), "board");
    gtk_grid_set_row_homogeneous(ui->grid, TRUE);
    gtk_grid_set_column_homogeneous(ui->grid, TRUE);

    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            GtkWidget *btn = gtk_button_new_with_label("");
            gtk_widget_set_size_request(btn, 44, 44);
            gtk_widget_add_css_class(btn, "cell");
            g_object_set_data(G_OBJECT(btn), "r", GINT_TO_POINTER(r));
            g_object_set_data(G_OBJECT(btn), "c", GINT_TO_POINTER(c));
            gtk_grid_attach(ui->grid, btn, c, r, 1, 1);
            g_signal_connect(btn, "clicked", G_CALLBACK(on_cell_clicked_internal), ui);
            ui->cells[r][c] = btn;
        }
    }

    gtk_grid_attach(GTK_GRID(frame), gtk_label_new(NULL), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(frame), top_letters,        1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(frame), gtk_label_new(NULL),2, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(frame), left_nums,          0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(frame), GTK_WIDGET(ui->grid),1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(frame), right_nums,         2, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(frame), gtk_label_new(NULL),0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(frame), bot_letters,        1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(frame), gtk_label_new(NULL),2, 2, 1, 1);

    build_victory_overlay(ui);
    build_sidebar(ui);

    gtk_box_append(GTK_BOX(root), ui->sidebar);
}

// -----------------------------------------------------------------------------
// Rafraîchissement
// -----------------------------------------------------------------------------


/**
 * @brief Met à jour l’affichage d’une case du plateau de jeu.
 * @fn refresh_cell_widget(UIState *ui, int r, int c, const GameState *game) 
 *
 * Cette fonction synchronise l’état visuel d’une cellule de la grille
 * avec l’état courant du jeu. Elle enlève d’abord toutes les classes CSS
 * existantes, puis applique celles qui correspondent à la situation :
 * - Couleur et type de pièce (roi ou soldat, bleu ou rouge).
 * - Ville bleue ou rouge.
 * - Contrôle de la case (bleu ou rouge).
 * - Occupation de la case (pièce présente).
 * - Sélection en cours.
 * - Marquage de la diagonale principale.
 *
 * Elle définit également le texte affiché dans le bouton correspondant :
 * - ♔ pour le roi bleu.
 * - ♖ pour le soldat bleu.
 * - ♚ pour le roi rouge.
 * - ♜ pour le soldat rouge.
 * - "C" pour une ville.
 *
 * @param ui   Pointeur vers la structure d’état de l’interface utilisateur.
 * @param r    Ligne de la case dans la grille.
 * @param c    Colonne de la case dans la grille.
 * @param game Pointeur vers l’état courant de la partie.
 */

static void refresh_cell_widget(UIState *ui, int r, int c, const GameState *game) {
    GtkWidget *w = ui->cells[r][c];
    const Square sq = game->board[r][c];

    const char *to_remove[] = {
        "blue", "red", "king", "soldier", "city-blue", "city-red",
        "selected", "ctrl-blue", "ctrl-red", "move-hint", "diag-yellow",
        "city-mark", "city-mark-blue", "city-mark-red", "occup-blue", "occup-red", NULL
    };
    for (int i = 0; to_remove[i]; ++i)
        if (w && GTK_IS_WIDGET(w)){
            gtk_widget_remove_css_class(w, to_remove[i]);
        }

    if (r + c == BOARD_SIZE - 1)
        gtk_widget_add_css_class(w, "diag-yellow");

    const char *txt = "";
    if (sq.piece != PIECE_NONE) {
        switch (sq.piece) {
            case PIECE_KING_BLUE:    txt = "♔"; break;
            case PIECE_SOLDIER_BLUE: txt = "♖"; break;
            case PIECE_KING_RED:     txt = "♚"; break;
            case PIECE_SOLDIER_RED:  txt = "♜"; break;
            default:                 txt = "";  break;
        }
    } else if (sq.cell == CELL_CITY_BLUE) {
        txt = "C";
        gtk_widget_add_css_class(w, "city-mark");
        gtk_widget_add_css_class(w, "city-mark-blue");
    } else if (sq.cell == CELL_CITY_RED) {
        txt = "C";
        gtk_widget_add_css_class(w, "city-mark");
        gtk_widget_add_css_class(w, "city-mark-red");
    }
    gtk_button_set_label(GTK_BUTTON(w), txt);

    if (sq.cell == CELL_CITY_BLUE) gtk_widget_add_css_class(w, "city-blue");
    if (sq.cell == CELL_CITY_RED)  gtk_widget_add_css_class(w, "city-red");

    if (sq.piece == PIECE_NONE) {
        if (game->ctrl[r][c] == CTRL_BLUE) gtk_widget_add_css_class(w, "ctrl-blue");
        else if (game->ctrl[r][c] == CTRL_RED) gtk_widget_add_css_class(w, "ctrl-red");
    }

    Player owner = game_piece_owner(sq.piece);
    if (owner == PLAYER_BLUE) {
        gtk_widget_add_css_class(w, "occup-blue");
        gtk_widget_add_css_class(w, "blue");
    } else if (owner == PLAYER_RED) {
        gtk_widget_add_css_class(w, "occup-red");
        gtk_widget_add_css_class(w, "red");
    }

    if (sq.piece == PIECE_KING_BLUE || sq.piece == PIECE_KING_RED)
        gtk_widget_add_css_class(w, "king");
    if (sq.piece == PIECE_SOLDIER_BLUE || sq.piece == PIECE_SOLDIER_RED)
        gtk_widget_add_css_class(w, "soldier");

    if (ui->have_selection && ui->sel_r == r && ui->sel_c == c)
        gtk_widget_add_css_class(w, "selected");
}


/**
 * @brief Met à jour l’affichage complet du plateau de jeu.
 * @fn ui_refresh_board(UIState *ui, const GameState *game)
 *
 * Cette fonction reconstruit l’interface si la fenêtre n’existe pas encore,
 * puis parcourt chaque case de la grille afin d’actualiser son état visuel
 * en fonction de l’état courant de la partie.
 *
 * @param ui   Pointeur vers la structure d’état de l’interface utilisateur.
 * @param game Pointeur vers l’état courant du jeu.
 */

void ui_refresh_board(UIState *ui, const GameState *game) {
    if (!ui || !game) return;
    if (!ui->win) build_ui_internal(ui);
    for (int r = 0; r < BOARD_SIZE; ++r)
        for (int c = 0; c < BOARD_SIZE; ++c)
            refresh_cell_widget(ui, r, c, game);
}

/**
 * @brief Met à jour l’affichage des scores des joueurs.
 * @fn ui_refresh_scores(UIState *ui, int blue, int red)
 *
 * Les étiquettes de score bleu et rouge sont mises à jour avec
 * les valeurs fournies.
 *
 * @param ui   Pointeur vers la structure d’état de l’interface utilisateur.
 * @param blue Score du joueur bleu.
 * @param red  Score du joueur rouge.
 */

void ui_refresh_scores(UIState *ui, int blue, int red) {
    if (!ui) return;
    char sbuf[64], rbuf[64];
    snprintf(sbuf, sizeof sbuf, "Bleu : %d", blue);
    snprintf(rbuf, sizeof rbuf, "Rouge : %d", red);
    gtk_label_set_text(GTK_LABEL(ui->score_blue_lbl), sbuf);
    gtk_label_set_text(GTK_LABEL(ui->score_red_lbl),  rbuf);
}

/**
 * @brief Détermine le joueur local en fonction du mode de jeu.
 * @fn Player local_player(const UIState *ui)
 *
 * - Si le mode est CLIENT, le joueur local est BLEU.
 * - Si le mode est SERVER, le joueur local est ROUGE.
 * - Sinon, aucun joueur local n’est attribué.
 *
 * @param ui Pointeur vers la structure d’état de l’interface utilisateur.
 * @return Joueur local (PLAYER_BLUE, PLAYER_RED ou PLAYER_NONE).
 */


static Player local_player(const UIState *ui) {
    if (ui->game_mode == MODE_CLIENT) return PLAYER_BLUE;
    if (ui->game_mode == MODE_SERVER) return PLAYER_RED;
    return PLAYER_NONE;
}


/**
 * @brief Met à jour l’affichage du panneau latéral (profil, tours, statut).
 * @fn ui_refresh_panel(UIState *ui, const GameState *game)
 * 
 * Cette fonction ajuste les textes et styles CSS des différentes sections :
 * - Profil : rôle du joueur local (Bleu, Rouge, ou mode IA/local).
 * - Tour courant : joueur dont c’est le tour.
 * - Tours restants : nombre de tours encore disponibles.
 * - Indication de statut : partie terminée, attente de l’adversaire,
 *   ou message "à vous de jouer".
 *
 * @param ui   Pointeur vers la structure d’état de l’interface utilisateur.
 * @param game Pointeur vers l’état courant du jeu.
 */

void ui_refresh_panel(UIState *ui, const GameState *game) {
    if (!ui || !game) return;

    if (ui->panel_role_lbl) {
        Player me = local_player(ui);
        const char *txt = NULL;
        if (me == PLAYER_BLUE) {
            txt = "Vous jouez : Joueur Bleu";
        } else if (me == PLAYER_RED) {
            txt = "Vous jouez : Joueur Rouge";
        } else if (ui->ia_enabled) {
            txt = "Mode local : joueur contre IA";
        } else {
            txt = "Mode local : deux joueurs";
        }
        gtk_label_set_text(GTK_LABEL(ui->panel_role_lbl), txt);
        if (ui->panel_role_lbl && GTK_IS_WIDGET(ui->panel_role_lbl)){
            gtk_widget_remove_css_class(ui->panel_role_lbl, "badge-blue");
            gtk_widget_remove_css_class(ui->panel_role_lbl, "badge-red");
        }
        if (me == PLAYER_BLUE) gtk_widget_add_css_class(ui->panel_role_lbl, "badge-blue");
        else if (me == PLAYER_RED) gtk_widget_add_css_class(ui->panel_role_lbl, "badge-red");
    }

    if (ui->panel_turn_lbl) {
        const char *who = game_player_name(game->turn);
        char buf[64];
        snprintf(buf, sizeof buf, "Tour actuel : %s", who);
        gtk_label_set_text(GTK_LABEL(ui->panel_turn_lbl), buf);
    }

    if (ui->panel_turns_lbl) {
        char buf[64];
        snprintf(buf, sizeof buf, "Tours restants : %d", game->turns_left);
        gtk_label_set_text(GTK_LABEL(ui->panel_turns_lbl), buf);
    }

    if (ui->panel_turn_hint) {
        if (ui->panel_turn_hint && GTK_IS_WIDGET(ui->panel_turn_hint)){
            gtk_widget_remove_css_class(ui->panel_turn_hint, "state-ok");
            gtk_widget_remove_css_class(ui->panel_turn_hint, "state-wait");
        }
        if (game->game_over) {
            gtk_label_set_text(GTK_LABEL(ui->panel_turn_hint), "Partie terminée");
            gtk_widget_add_css_class(ui->panel_turn_hint, "state-wait");
        } else if (ui->game_mode == MODE_LOCAL) {
            char txt[96];
            snprintf(txt, sizeof txt, "À vous de jouer : %s", game_player_name(game->turn));
            gtk_label_set_text(GTK_LABEL(ui->panel_turn_hint), txt);
            gtk_widget_add_css_class(ui->panel_turn_hint, "state-ok");
        } else {
            Player me = local_player(ui);
            if ((ui->game_mode == MODE_CLIENT && game->turn == PLAYER_BLUE) ||
                (ui->game_mode == MODE_SERVER && game->turn == PLAYER_RED)) {
                gtk_label_set_text(GTK_LABEL(ui->panel_turn_hint), "C'est à votre tour");
                gtk_widget_add_css_class(ui->panel_turn_hint, "state-ok");
            } else {
                const char *opp = (me == PLAYER_BLUE) ? "Joueur Rouge" : "Joueur Bleu";
                char txt[96];
                snprintf(txt, sizeof txt, "%s joue…", opp);
                gtk_label_set_text(GTK_LABEL(ui->panel_turn_hint), txt);
                gtk_widget_add_css_class(ui->panel_turn_hint, "state-wait");
            }
        }
    }
}


/**
 * @brief Met à jour le texte de la zone de statut.
 * @fn ui_set_status(UIState *ui, const char *text)
 *
 * @param ui   Pointeur vers la structure d’état de l’interface utilisateur.
 * @param text Nouveau texte à afficher (chaîne vide si NULL).
 */

void ui_set_status(UIState *ui, const char *text) {
    if (!ui || !ui->status_lbl) return;
    gtk_label_set_text(GTK_LABEL(ui->status_lbl), text ? text : "");
}

/**
 * @brief Met en évidence une case comme étant sélectionnée.
 * @fn ui_show_selection(UIState *ui, int r, int c)
 *
 * Supprime toute sélection précédente puis marque la nouvelle case
 * avec la classe CSS "selected".
 *
 * @param ui Pointeur vers la structure d’état de l’interface utilisateur.
 * @param r  Ligne de la case sélectionnée.
 * @param c  Colonne de la case sélectionnée.
 */

void ui_show_selection(UIState *ui, int r, int c) {
    if (!ui) return;
    ui_clear_selection(ui);
    ui->have_selection = true;
    ui->sel_r = r;
    ui->sel_c = c;
    gtk_widget_add_css_class(ui->cells[r][c], "selected");
}


/**
 * @brief Supprime la sélection en cours (s’il y en a une).
 * @fn ui_clear_selection(UIState *ui)
 *
 * Retire la classe CSS "selected" de la case actuellement sélectionnée.
 *
 * @param ui Pointeur vers la structure d’état de l’interface utilisateur.
 */

void ui_clear_selection(UIState *ui) {
    if (!ui || !ui->have_selection) return;
    if (ui->cells[ui->sel_r][ui->sel_c] && GTK_IS_WIDGET(ui->cells[ui->sel_r][ui->sel_c])) {
        gtk_widget_remove_css_class(ui->cells[ui->sel_r][ui->sel_c], "selected");
    }
    ui->have_selection = false;
}


/**
 * @brief Efface toutes les indications de coups possibles (move hints).
 * @fn ui_clear_move_hints(UIState *ui)
 *
 * Supprime le timer associé aux indices, enlève la classe CSS "move-hint"
 * de toutes les cases, et réinitialise la liste des widgets de suggestion.
 *
 * @param ui Pointeur vers la structure d’état de l’interface utilisateur.
 */

void ui_clear_move_hints(UIState *ui) {
    if (!ui) return;
    if (ui->hint_timer_id) {
        g_source_remove(ui->hint_timer_id);
        ui->hint_timer_id = 0;
    }
    for (int r = 0; r < BOARD_SIZE; ++r)
        for (int c = 0; c < BOARD_SIZE; ++c)
            if (ui->cells[r][c] && GTK_IS_WIDGET(ui->cells[r][c])) {
                gtk_widget_remove_css_class(ui->cells[r][c], "move-hint");
            }
    if (ui->hint_widgets) g_ptr_array_set_size(ui->hint_widgets, 0);
    ui->hint_index = 0;
}


/**
 *  @struct HintCel
 *  @brief Structure représentant une case de suggestion de coup.
 *  @details Contient la ligne, la colonne et la distance par rapport
 *  à la pièce sélectionnée, pour ordonner les coups proposés.
 */
typedef struct { 
    int r;   /** < Ligne de la case. */
    int c;   /** < Colonne de la case. */
    int dist;/** < Distance Manhattan par rapport à la pièce d'origine. */
} HintCell;

/** 
 *  @fn int hint_cell_cmp(gconstpointer a, gconstpointer b)
 *  @brief Fonction de comparaison de deux HintCell.
 *  @param a pointeur vers un HintCell
 *  @param b pointeur vers un HintCell
 *  @return valeur négative si a < b, positive si a > b, 0 si égalité.
 */
static int hint_cell_cmp(gconstpointer a, gconstpointer b) {
    const HintCell *x = (const HintCell*)a;
    const HintCell *y = (const HintCell*)b;
    if (x->dist != y->dist) return x->dist - y->dist;
    if (x->r != y->r) return x->r - y->r;
    return x->c - y->c;
}


/** 
 *  @fn gboolean hint_tick_cb(gpointer data)
 *  @brief Callback appelé périodiquement pour afficher les coups possibles.
 *  @param data pointeur vers l'UIState
 *  @return G_SOURCE_CONTINUE pour continuer, G_SOURCE_REMOVE pour arrêter.
 */
static gboolean hint_tick_cb(gpointer data) {
    UIState *ui = (UIState*)data;
    if (!ui || !ui->hint_widgets) return G_SOURCE_REMOVE;
    if (ui->hint_index >= (int)ui->hint_widgets->len) {
        ui->hint_timer_id = 0;
        return G_SOURCE_REMOVE;
    }
    GtkWidget *w = GTK_WIDGET(g_ptr_array_index(ui->hint_widgets, ui->hint_index++));
    if (w) gtk_widget_add_css_class(w, "move-hint");
    return G_SOURCE_CONTINUE;
}


/**
 *  @fn ui_show_move_hints(UIState *ui, const GameState *game, int r, int c)
 *  @brief Affiche des indices visuels pour les coups possibles d'une pièce.
 *  @param ui état de l'interface graphique
 *  @param game état du jeu
 *  @param r ligne de la pièce sélectionnée
 *  @param c colonne de la pièce sélectionnée
 */
void ui_show_move_hints(UIState *ui, const GameState *game, int r, int c) {
    if (!ui || !game) return;
    ui_clear_move_hints(ui);

    int moves[BOARD_SIZE * 2][2];
    int count = 0;
    game_get_legal_moves(game, r, c, moves, &count);
    if (count <= 0) return;

    GArray *ordered = g_array_sized_new(FALSE, FALSE, sizeof(HintCell), count);
    for (int i = 0; i < count; ++i) {
        HintCell cell = {
            .r = moves[i][0],
            .c = moves[i][1],
            .dist = abs(moves[i][0] - r) + abs(moves[i][1] - c)
        };
        g_array_append_val(ordered, cell);
    }
    g_array_sort(ordered, hint_cell_cmp);

    for (guint i = 0; i < ordered->len; ++i) {
        HintCell *cell = &g_array_index(ordered, HintCell, i);
        GtkWidget *w = ui->cells[cell->r][cell->c];
        g_ptr_array_add(ui->hint_widgets, w);
    }
    g_array_unref(ordered);

    ui->hint_index = 0;
    hint_tick_cb(ui);
    ui->hint_timer_id = g_timeout_add(70, hint_tick_cb, ui);
}


/**
 *  @fn ui_show_victory(UIState *ui, Player winner, const char *reason)
 *  @brief Affiche l'écran de victoire.
 *  @param ui état de l'interface graphique
 *  @param winner joueur gagnant
 *  @param reason raison de la victoire (chaîne libre)
 */
void ui_show_victory(UIState *ui, Player winner, const char *reason) {
    if (!ui) return;
    const char *who = game_player_name(winner);
    char msg[128];
    snprintf(msg, sizeof msg, "%s a gagné !", who);
    gtk_label_set_text(GTK_LABEL(ui->victory_label), msg);

    if (ui->game_mode != MODE_LOCAL && ui->defeat_label && GTK_IS_WIDGET(ui->defeat_label)) {
        if (winner == PLAYER_BLUE)
            gtk_label_set_text(GTK_LABEL(ui->defeat_label), "Le joueur Rouge a perdu");
        else
            gtk_label_set_text(GTK_LABEL(ui->defeat_label), "Le joueur Bleu a perdu");
        gtk_widget_set_visible(ui->defeat_label, TRUE);
    } else {
        gtk_widget_set_visible(ui->defeat_label, FALSE);
    }

    printf("=== FIN DE PARTIE : %s (%s) ===\n", msg, reason ? reason : "—");

    if (ui->victory_layer && GTK_IS_WIDGET(ui->victory_layer))
        gtk_widget_set_visible(ui->victory_layer, TRUE);
    start_confetti(ui);
}

/**
 *  @fn ui_show_draw(UIState *ui, const char *reason)
 *  @brief Affiche l'écran d'égalité.
 *  @param ui état de l'interface graphique
 *  @param reason raison du match nul
 */
void ui_show_draw(UIState *ui, const char *reason) {
    if (!ui) return;
    if (ui->defeat_label && GTK_IS_WIDGET(ui->defeat_label))
        gtk_widget_set_visible(ui->defeat_label, FALSE);
    if (ui->victory_label && GTK_IS_WIDGET(ui->victory_label))
        gtk_label_set_text(GTK_LABEL(ui->victory_label), "Égalité !");
    printf("=== FIN DE PARTIE : Égalité (%s) ===\n", reason ? reason : "—");

    if (ui->victory_layer && GTK_IS_WIDGET(ui->victory_layer))
        gtk_widget_set_visible(ui->victory_layer, TRUE);
    if (ui->confetti_area && GTK_IS_WIDGET(ui->confetti_area))
        gtk_widget_set_visible(ui->confetti_area, FALSE);
}

/**
 *  @fn ui_hide_victory_overlay(UIState *ui)
 *  @brief Cache l'écran de victoire et arrête les confettis.
 *  @param ui état de l'interface graphique
 */
void ui_hide_victory_overlay(UIState *ui) {
    if (!ui) return;
    stop_confetti(ui);
    if (ui->victory_layer && GTK_IS_WIDGET(ui->victory_layer))
        gtk_widget_set_visible(ui->victory_layer, FALSE);
}

/**
 *  @fn ui_set_board_sensitive(UIState *ui, bool sensitive)
 *  @brief Rend le plateau cliquable ou non.
 *  @param ui état de l'interface graphique
 *  @param sensitive true pour activer, false pour désactiver
 */

void ui_set_board_sensitive(UIState *ui, bool sensitive) {
    if (!ui) return;
    for (int r = 0; r < BOARD_SIZE; ++r)
        for (int c = 0; c < BOARD_SIZE; ++c)
            gtk_widget_set_sensitive(ui->cells[r][c], sensitive);
}

/**
 *  @fn ui_set_replay_status(UIState *ui, int ready_count)
 *  @brief Met à jour l'état de "rejouer" pour les parties en ligne.
 *  @param ui état de l'interface graphique
 *  @param ready_count nombre de joueurs prêts
 */

void ui_set_replay_status(UIState *ui, int ready_count) {
    if (!ui) return;
    if (ui->game_mode == MODE_LOCAL) {
        if (ui->replay_status_overlay && GTK_IS_WIDGET(ui->replay_status_overlay))
            gtk_widget_set_visible(ui->replay_status_overlay, FALSE);
        return;
    }
    char msg[64];
    snprintf(msg, sizeof msg, "Joueur voulant relancer la partie %d/2", ready_count);
    if (ui->replay_status_overlay && GTK_IS_WIDGET(ui->replay_status_overlay)) {
        gtk_widget_set_visible(ui->replay_status_overlay, TRUE);
        gtk_label_set_text(GTK_LABEL(ui->replay_status_overlay), msg);
    }
    if (ui->panel_replay_status && GTK_IS_WIDGET(ui->panel_replay_status))
        gtk_label_set_text(GTK_LABEL(ui->panel_replay_status), msg);
}

// -----------------------------------------------------------------------------
// API publique
// -----------------------------------------------------------------------------


/**
 * @fn UIState* ui_create(GameMode mode, bool ia_enabled)
 * @brief Crée et initialise une structure UIState.
 * @param mode mode de jeu
 * @param ia_enabled true si une IA est activée
 * @return pointeur vers une nouvelle structure UIState
 */
UIState* ui_create(GameMode mode, bool ia_enabled) {
    UIState *ui = g_new0(UIState, 1);
    ui->game_mode = mode;
    ui->ia_enabled = ia_enabled;
    ui->have_selection = false;
    ui->sel_r = ui->sel_c = -1;
    ui->hint_widgets = g_ptr_array_new_with_free_func(NULL);
    return ui;
}

/** 
 *  @fn ui_destroy(UIState *ui)
 *  @brief Libère la mémoire associée à une structure UIState.
 *  @param ui pointeur vers l'UIState à détruire
 */
void ui_destroy(UIState *ui) {
    if (!ui) return;
    ui_clear_move_hints(ui);
    stop_confetti(ui);
    if (ui->hint_widgets) g_ptr_array_unref(ui->hint_widgets);
    g_free(ui);
}


/**
 *  @fn ui_set_callbacks(UIState *ui, UICallbacks callbacks)
 *  @brief Associe les callbacks de l'interface.
 *  @param ui état de l'interface graphique
 *  @param callbacks structure contenant les callbacks
 */
void ui_set_callbacks(UIState *ui, UICallbacks callbacks) {
    if (!ui) return;
    ui->callbacks = callbacks;
}

/**
 *  @fn GtkWindow* ui_get_window(UIState *ui)
 *  @brief Retourne la fenêtre principale de l'interface.
 *  @param ui état de l'interface graphique
 *  @return pointeur vers GtkWindow
 */
GtkWindow* ui_get_window(UIState *ui) {
    if (!ui) return NULL;
    if (!ui->win) build_ui_internal(ui);
    return ui->win;
}

/** 
 *  @fn ui_show(UIState *ui)
 *  @brief Affiche la fenêtre principale.
 *  @param ui état de l'interface graphique
 */
void ui_show(UIState *ui) {
    GtkWindow *win = ui_get_window(ui);
    if (win) gtk_window_present(win);
}


/**
 * @fn ui_load_css(void)
 * @brief Charge le fichier CSS de style de l'interface.
 * @details Cherche dans `data/style.css` puis `../data/style.css`.
 */
void ui_load_css(void) {
    GtkCssProvider *provider = gtk_css_provider_new();
    GFile *f1 = g_file_new_for_path("data/style.css");
    GFile *f2 = g_file_new_for_path("../data/style.css");

    if (g_file_query_exists(f1, NULL)) {
        gtk_css_provider_load_from_file(provider, f1);
    } else if (g_file_query_exists(f2, NULL)) {
        gtk_css_provider_load_from_file(provider, f2);
    } else {
        g_warning("Fichier CSS introuvable (data/style.css)");
    }

    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(), GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER);

    g_object_unref(f1);
    g_object_unref(f2);
    g_object_unref(provider);
}
