#ifndef CLIENT_H
#define CLIENT_H
/**
 * @file client.h
 * @brief Déclarations pour la gestion du client réseau du jeu Krojanty
 * @author kylian.adam@uha.fr
 * @note Fichier documenté par hadil.abahmane@uha.fr
 */

/**
*@struct ClientContext
*@brief Structure contenant le contexte de connexion du client 
*/
typedef struct {
    int socket;
    int port;
    char ip[16]; // Ex : "127.0.0.1"
} ClientContext;


/**
*@fn void client_init(ClientContext *ctx, const char *ip, unsigned short port)
*@brief Initialise le client et connecte au serveur
*@param ctx Contexte du client à initialiser
*@param ip Adresse IP du serveur à connecter
*@param port Port de connexion du serveur
*/ 
void client_init(ClientContext *ctx, const char *ip, unsigned short port);


/** 
*@fn void client_close(ClientContext *ctx)
*@brief Ferme proprement la connexion
*@param ctx Contexte du client à Fermer
*/
void client_close(ClientContext *ctx);


/**
*@fn void client_send_message(ClientContext *ctx, const char *message)
*@brief Envoie un message au serveur
*@param ctx Contexte de connexion du client
*@param message Message à envoyer (doit faire exactement 4octets)
*/
void client_send_message(ClientContext *ctx, const char *message);


/**
*@fn char *client_receive_message(ClientContext *ctx)
*@brief Reçoit un message (UTF-8 et 4octets)
*@param ctx Contexte de connexion du client 
*@return BUffer contenant le message reçu
*/
char *client_receive_message(ClientContext *ctx);


/**
*@fn _Bool is_server_alive(int sock)
*@brief Vérifie si le socket est vivant 
*@param sock Descripteur de socket à vérifier
*@return 1 si le serveur est accessible, 0 sinon
*/ 
_Bool is_server_alive(int sock);

#endif
