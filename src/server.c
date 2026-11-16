/** 
 * @file server.c
 * @brief Implementation des fonctions de gestion du serveur réseau pour le jeu Krojanty 
 * @author kylian.adam@uha.fr 
 *@note Fichier documenté par hadil.abahmane@uha.fr
 */

#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

/** 
* \fn _Bool is_client_alive(int sock)
* \brief Vérifie si le client connecté sur le socket est toujours en vie
* \param sock Le descripteur de socket à vérifier
* \return 1 si le client est vivant, 0 s'il est déconnecté ou en erreur
*/

_Bool is_client_alive(int sock) {
    char tmp;
    int res = recv(sock, &tmp, 1, MSG_PEEK | MSG_DONTWAIT);
    if (res == 0) return 0;
    if (res < 0 && errno != EAGAIN && errno != EWOULDBLOCK) return 0;
    return 1;
}
/**
* \fn void server_init(ServerContext *ctx, unsigned short port)
* \brief Initialise le serveur, l'attache au port spécialisé et attend une connexion client
* \param ctx Pointeur vers la structure ServerContext à initialiser
* \param port le port TCP sur lequel le serveur doit écouter (de 0 à 65535)
* \note En cas d'erreur, la fonction termine le programme avec exit(EXIT_FAILURE)
* \note Le serveur accepte une seule connexion client à la fois
* \note Le socket du serveur est configuré pour réutiliser l'adresse et pour se fermer immédiatement à la fermeture
*/

void server_init(ServerContext *ctx, unsigned short port) {
    int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serv_sock < 0) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    _Bool opt = 1;
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct linger sl = {1, 0}; // Linger active, timeout 0
    setsockopt(serv_sock, SOL_SOCKET, SO_LINGER, &sl, sizeof(sl));

    struct sockaddr_in serv_addr = {0};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind()");
        close(serv_sock);
        exit(EXIT_FAILURE);
    }

    if (listen(serv_sock, 1) < 0) {
        perror("listen()");
        close(serv_sock);
        exit(EXIT_FAILURE);
    }

    printf("[INFO] Serveur en attente sur le port %d...\n", port);

    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    int client_sock = accept(serv_sock, (struct sockaddr *)&client_addr, &len);
    if (client_sock < 0) {
        perror("accept()");
        close(serv_sock);
        exit(EXIT_FAILURE);
    }

    close(serv_sock);

    ctx->socket = client_sock;
    ctx->port = port;

    printf("[INFO] Client connecté.\n");
}

/** 
*\fn void close_server(ServerContext *ctx)
*\brief Ferme proprement la connexion serveur
*\param ctx Pointeur vers la structure ServerContext contenant la connexion à fermer
*/

void close_server(ServerContext *ctx) {
    if (ctx->socket >= 0) {
        close(ctx->socket);
        ctx->socket = -1;
    }
}
/** 
*\fn void send_message(ServerContext *ctx, const char *message)
*\brief Envoie un message de taille fixe (4octets) au client
*\param ctx Pointeur vers une chaine d'au moins 4octets à envoyer
*/

void send_message(ServerContext *ctx, const char *message) {
    if (!is_client_alive(ctx->socket)) {
        printf("[WARN] Socket mort. Reconnexion...\n");
        close_server(ctx);
        server_init(ctx, ctx->port);
    }

    send(ctx->socket, message, 4, 0);
}

/** 
*\fn char *receive_message(ServerContext *ctx)
*\brief Reçoit un message de taille fixe (4octets) du client
*\param ctx Pointeur vers la structure ServerContext contenant la connexion
*\return Pointeur vers une chaine de 4caracteres + '\0'  ou NULL en cas d'erreur
*/

char *receive_message(ServerContext *ctx) {
    if (!is_client_alive(ctx->socket)) {
        printf("[WARN] Socket mort. Reconnexion...\n");
        close_server(ctx);
        server_init(ctx, ctx->port);
    }

    char buffer[4];
    ssize_t bytes_read = recv(ctx->socket, buffer, 4, MSG_WAITALL);
    printf("[DEBUG SERVEUR] Reçu %zd octets\n", bytes_read);

    if (bytes_read != 4) {
        return NULL;
    }

    printf("[DEBUG SERVEUR] Message : %.*s\n", 4, buffer);

    char *result = malloc(5);
    if (!result) return NULL;
    memcpy(result, buffer, 4);
    result[4] = '\0';
    return result;
}
