/**
*@file server.h
*@brief Déclarations pour la gestion du serveur réseau du jeu Krojanty
*@author kylian.adam@uha.fr
*@note  Fichier documenté par hadil.abahmane@uha.fr
*/

#ifndef SERVER_H
#define SERVER_H

/**
*\struct ServerConetxt
*\brief Structure contenant le contexte de connexion du serveur
*/

typedef struct {
    int socket;
    unsigned short port;
} ServerContext;

/**
*\fn void server_init(ServerContext *ctx, unsigned short port)
*\brief Initialise le serveur(ouvre socket, accepte un client)
*\param ctx Contexte du serveur à initialiser
*\param port Port d'écoute du serveur
*/

void server_init(ServerContext *ctx, unsigned short port);

/**
*\fn void close_server(ServerContext *ctx)
*\brief Ferme proprement le socket du serveur
*\param ctx Contexte du serveur à fermer
*/

void close_server(ServerContext *ctx);

/**
*\fn void send_message(ServerContext *ctx, const char *message)
*\brief Envoie un message au client
*\param ctx Contexte de connexion du serveur
*\param message Message à envoyer (doit faire exactement 4octets)
*/
void send_message(ServerContext *ctx, const char *message);

/**
*\fn char *receive_message(ServerContext *ctx)
*\brief Reçoit un message (renvoie un buffer de 4octets UTF-8)
*\param ctx Contexte de connexion du serveur
*\return Buffer contenant le message reçu
*/
char *receive_message(ServerContext *ctx);

/**
*\fn _Bool is_client_alive(int sock)
*\brief Vérifie si le socket est vivant
*\param sock Descripteur de socket à vérifier
*\return 1 si le client est connecté, 0 sinon
*/
_Bool is_client_alive(int sock);

#endif
