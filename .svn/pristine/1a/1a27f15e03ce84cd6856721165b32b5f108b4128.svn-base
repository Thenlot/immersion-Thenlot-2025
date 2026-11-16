/**
 * @file client.c
 * @brief Implementation des fonctions de gestion du client réseau pour le jeu Krojanty
 * @author kylian.adam@uha.fr
 * @note Fichier documenté par hadil.abahmane@uha.fr
 */

#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>

/**
 * @fn is_server_alive(int sock)
 * @brief Vérifie si la connexion au serveur est toujours active
 * @param sock Le descripteur de socket à vérifier
 * @return 1 si le serveur est accessible, 0 si déconnecté ou y a une erreur
 */

_Bool is_server_alive(int sock) {
    char tmp;
    int res = recv(sock, &tmp, 1, MSG_PEEK | MSG_DONTWAIT);
    if (res == 0) return 0;
    if (res < 0 && errno != EAGAIN && errno != EWOULDBLOCK) return 0;
    return 1;
}

/**
 * @fn client_init(ClientContext *ctx, const char *ip, int port)
 * @brief Initialise le client et établit la connexion au serveur
 * @param ctx Pointeur vers la structure ClientContext à initialiser
 * @param ip L'adresse IP du serveur à connecter
 * @param port Port du serveur à connecter
 * @note la fonction affiche l'erreur et termine le processus avec EXIT_FAILURE
 */

void client_init(ClientContext *ctx, const char *ip, unsigned short port) {
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serv_addr = {0};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        perror("inet_pton()");
        close(sock);
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect()");
        close(sock);
        exit(EXIT_FAILURE);
    }

    ctx->socket = sock;
    ctx->port = port;
    strncpy(ctx->ip, ip, sizeof(ctx->ip) - 1);
    ctx->ip[sizeof(ctx->ip) - 1] = '\0';

    printf("[INFO] Connecté à %s:%d\n", ctx->ip, ctx->port);
}

/**
 * @fn client_close(ClientContext *ctx)
 * @brief Ferme proprement la connexion client
 * @param ctx Pointeur vers la structure ClientContext contenant la connexion à fermer
 */

void client_close(ClientContext *ctx) {
    if (ctx->socket >= 0) {
        close(ctx->socket);
        ctx->socket = -1;
    }
}

/**
 * @fn client_send_message(ClientContext *ctx, const char *message)
 * @brief Envoie un message de taille fixe (4octets) au serveur
 * @param ctx Pointeur vers la structure ClientContext contenant la connexion
 * @param message Pointeur vers une chaine d'au moins 4octets à envoyer
 * @warning La fonction suppose que le message à envoyer fait exactement 4octets
 */

void client_send_message(ClientContext *ctx, const char *message) {
    if (!is_server_alive(ctx->socket)) {
        printf("[WARN] Socket mort. Reconnexion...\n");
        client_close(ctx);
        client_init(ctx, ctx->ip, ctx->port);
    }
    printf("[DEBUG CLIENT] Envoi : %.*s\n", 4, message);
    send(ctx->socket, message, 4, 0);
}

/**
 * @fn client_receive_message(ClientContext *ctx)
 * @brief Reçoit un message de taille fixe (4octets) du serveur
 * @param ctx Pointeur vers la structure ClientContext contenant la connexion
 * @return Pointeur vers une chaine de 5octets (4octets + '\0') ou NULL en cas d'erreur
 */

char *client_receive_message(ClientContext *ctx) {
    if (!is_server_alive(ctx->socket)) {
        printf("[WARN] Socket mort. Reconnexion...\n");
        client_close(ctx);
        client_init(ctx, ctx->ip, ctx->port);
    }

    char buffer[4];
    ssize_t bytes_read = recv(ctx->socket, buffer, 4, MSG_WAITALL);

    if (bytes_read != 4) return NULL;

    printf("[DEBUG CLIENT] Message : %.*s\n", 4, buffer);

    char *result = malloc(5);
    if (!result) return NULL;
    memcpy(result, buffer, 4);
    result[4] = '\0';
    return result;
}
