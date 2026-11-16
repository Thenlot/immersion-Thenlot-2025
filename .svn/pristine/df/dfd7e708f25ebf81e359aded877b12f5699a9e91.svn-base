/**
 * @file com.test.c
 * @brief Tests de communication entre un client et un serveur.
 * @author kylian.adam@uha.fr
 */

#include "../src/client.h"
#include "../src/server.h"
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "com.test.h"

/**
 * @brief Structure pour passer les arguments au thread serveur.
 */
typedef struct {
    ServerContext *ctx;
    unsigned short port;
} ServerThreadArgs;

/**
 * @brief Fonction exécutée par le thread serveur.
 * @param arg Pointeur vers les arguments du thread (ServerThreadArgs).
 */
void *server_thread_func(void *arg) {
    ServerThreadArgs *args = (ServerThreadArgs *)arg;
    server_init(args->ctx, args->port);
    usleep(100000);
    close_server(args->ctx);
    char *msg = receive_message(args->ctx);
    if (msg) {
        printf("Serveur a reçu : %s\n", msg);
        // Le serveur répond au client
        close_server(args->ctx);
        send_message(args->ctx, "PONG");
        free(msg);
    } else {
        printf("Serveur : aucun message reçu.\n");
    }
    return NULL;
}

/**
 * @brief Teste la communication entre un client et un serveur.
 * @return 0 si le test réussit, 1 sinon.
 * @note on test ici sur le port 8080, assurez-vous qu'il est libre.
 */
int batch_tests_com() {
    printf("=== Démarrage du test de communication ===\n");
    ServerContext server_ctx;
    ServerThreadArgs args = { .ctx = &server_ctx, .port = 8080 };

    // Démarrage du thread serveur
    pthread_t server_thread;
    if (pthread_create(&server_thread, NULL, server_thread_func, &args) != 0) {
        fprintf(stderr, "Erreur pthread_create (serveur)\n");
        return 1;
    }

    // Petit délai pour laisser le serveur prêt
    usleep(100000);

    ClientContext client_ctx;
    client_init(&client_ctx, "127.0.0.1", 8080);
    usleep(200000);
    client_send_message(&client_ctx, "PING");

    // Le client attend la réponse du serveur
    client_receive_message(&client_ctx);
    char *response = client_receive_message(&client_ctx);
    if (response) {
        printf("Client a reçu : %s\n", response);
        free(response);
    } else {
        printf("Client : aucune réponse reçue.\n");
    }

    client_close(&client_ctx);

    pthread_join(server_thread, NULL);
    close_server(&server_ctx);

    printf("=== Test de communication terminé avec succès ===\n");
    return 0;
}
