/** 
 * @file main.c
 * @brief Point d'entrée principal avec parsing des arguments
 * @author sofiane.bouguerri@uha.fr
  *@note Fichier documenté par hadil.abahmane@uha.fr
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include <gtk/gtk.h>

GtkApplication *g_app_forced_quit = NULL;

/**
 * @fn launch(int argc, char *argv[])
 * @brief Point d'entrée principal avec parsing des arguments
 * @param argc Nombre d'arguments
 * @param argv Arguments de ligne de commande
 * @return Code de sortie de l'application
 */
int launch(int argc, char *argv[]) {
    int ia_enabled = 0;
    if (argc < 2) {
        printf("Usage: %s [options] More\n", argv[0]);
        printf(" -l   Launching app locally (can be used followed with -ia to play against the IA)\n");
        printf(" -s   Launching app on server mode (must be followed with a port PPPP, can be launched followed with -ia)\n");
        printf(" -c   Launching app on client mode (must be followed with an ip and a port X.X.X.X:PPPP, can be launched followed with -ia)\n");
        printf(" -ia  Adding the ia on the app\n");
        return 1;
    }
    
    /** @note Si -l utilisé passage en mode local */
    if (strcmp(argv[1], "-l") == 0) {
        /**Mode local*/
        printf("Local Mode\n");
        if (argc > 3) {
            printf("Error: only -ia can follow -l\n");
            return 1;
        }
        /** Permet d'annoncer le mode IA en local ou bien de préciser que l'argument n'est pas bon si c'est le cas*/
        if (argc == 3 && strcmp(argv[2], "-ia") == 0) {
            printf("IA mode enabled\n");
            ia_enabled = 1;
        } else if (argc == 3) {
            printf("Error : invalid arguments following -l : %s\n", argv[2]);
            return 1;
        }
        return play_local(ia_enabled);
    }
    /** @note  Si -s est utilisé, passage en mode serveur en attente d'une connection (l'utilisation du -ia est possible pour que le serveur soit joué par l'ia)*/
    else if (strcmp(argv[1], "-s") == 0) {
        /** Mode serveur 
          @warning (force l'utilisation d'un port car en passage mode serveur il faut préciser sur quel port le serveur va ouvrir les sockets
         */
        if (argc < 3) {
            printf("Error : -s must be followed with a port\n");
            return 1;
        }
        if (!is_number(argv[2])) {
            printf("Error : invalid port following -s : %s\n", argv[2]);
            return 1;
        }
        printf("Server Mode on port : %s\n", argv[2]);

        /**Gestion du mode IA en Serveur*/
        if (argc == 4) {
            if (strcmp(argv[3], "-ia") == 0) {
                printf("IA Mode enabled with server mode\n");
                ia_enabled = 1;
            } else {
                printf("Error : Invalid argument following port : %s\n", argv[3]);
                return 1;
            }
        } else if (argc > 4) {
            printf("Error : Too much arguments following -s port\n");
            return 1;
        }
        return play_server(ia_enabled, atoi(argv[2]));
    }
    /** Prise en charge du mode client 
      @warning (demande obligatoirement une adresse ip au format X.X.X.X et son port en :PPPP)
     */
    else if (strcmp(argv[1], "-c") == 0) {
        /**Mode client*/
        if (argc < 3) {
            printf("Error : -c must be followed by IP:PORT\n");
            return 1;
        }
        if (!validate_ip_port(argv[2])) {
            printf("Error : format IP:PORT invalid : %s\n", argv[2]);
            return 1;
        }
        printf("Client mode, connexion to : %s\n", argv[2]);
        if (argc == 4) {
            if (strcmp(argv[3], "-ia") == 0) {
                printf("AI mode enabled with client mode\n");
                ia_enabled = 1;
            } else {
                printf("Error : Invalid argument following IP:PORT : %s\n", argv[3]);
                return 1;
            }
        } else if (argc > 4) {
            printf("Error : too much arguments following IP:PORT\n");
            return 1;
        }
        char ip_port_copy[64];
        strncpy(ip_port_copy, argv[2], sizeof(ip_port_copy));
        ip_port_copy[sizeof(ip_port_copy)-1] = '\0';
        char *ip = strtok(ip_port_copy, ":");
        int port = atoi(strtok(NULL, ":"));
        return play_client(ia_enabled, ip, port);
    }
    /**Empêche l'utilisation de tout autre type d'entrée.*/
    else {
        printf("Error : Initial argument invalid : %s\n", argv[1]);
        return 1;
    }

    return 0;
}

void force_quit_gtk(void) {
    if (g_app_forced_quit)
        g_application_quit(G_APPLICATION(g_app_forced_quit));
}

// plus pratique pour qu'il n'y ait pas 2 main pour les tests
#ifndef TEST
int main(int argc, char *argv[]){
    return launch(argc, argv);
}
#endif


/** @note Ce programme a pour but d'offrir à l'utilisateur une intéraction dans le CLI pour executer notre application. Avec un -l pour un mode local J1 vs J2 sur le même écran
 * mais aussi un -s pour entrer en mode serveur afin d'héberger une partie et d'attendre la connexion d'une autre application et enfin du -c pour afin de passer en mode client
 * et de se connecter à un serveur distant. Dans chacun de ces modes il est possible/nécessaires parfois d'ajouter des arguments supplémentaires (pour -l le -ia est optionnel,
 * pour -s le port est obligatoire et le -ia optionnel, pour -c l'ip:port est obligatoire et le -ia optionnel). Toute erreur dans les arguments fournis est gérée et un message d'erreur est affiché.
 * Le programme se charge ensuite d'appeler la fonction adéquate en fonction du mode choisi avec les bons paramètres.
*/