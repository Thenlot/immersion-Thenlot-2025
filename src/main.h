/** 
*@file main.h
*@brief Déclarations des fonctions principales pour le lancement du jeu Krojanty
*@author kylian.adam@uha.fr
*@note Fichier documenté par hadil.abahmane@uha.fr
*/

#ifndef MAIN_H
#define MAIN_H

#include "game.h"
#include <gtk/gtk.h>

/**
*@fn int play_local (_Bool ia_enabled)
*@brief Démarrer la partie en mode local
*@param ia_enabled Booléen indiquant si l'IA est activée
*/
int play_local(_Bool ia_enabled);

/**
 * @fn int play_client(_Bool ia_enabled, char *ip, unsigned short port)
 * @brief Démarrer la partie en mode client
 * @param ia_enabled Booléen indiquant si l'IA est activée
 * @param ip Adresse IP du serveur
 * @param port Port du serveur
 */
int play_client(_Bool ia_enabled, char *ip, unsigned short port);

/**
 * @fn int play_server(_Bool ia_enabled, unsigned short port)
 * @brief Démarrer la partie en mode serveur
 * @param ia_enabled Booléen indiquant si l'IA est activée
 * @param port Port sur lequel le serveur écoute
 */
int play_server(_Bool ia_enabled, unsigned short port);

/**
 * @fn int is_number(const char *str)
 * @brief Vérifie si une chaîne ne contient que des chiffres
 * @param str Chaîne à vérifier
 * @return 1 si la chaîne ne contient que des chiffres, 0 sinon
 */
int is_number(const char *str);

/**
 * @fn int validate_ip_port(const char *ip_port)
 * @brief Vérifie le format IP:PORT
 * @param ip_port Chaîne au format IP:PORT à valider
 * @return 1 si le format est valide, 0 sinon
 */
int validate_ip_port(const char *ip_port);

extern GtkApplication *g_app_forced_quit;

#endif
