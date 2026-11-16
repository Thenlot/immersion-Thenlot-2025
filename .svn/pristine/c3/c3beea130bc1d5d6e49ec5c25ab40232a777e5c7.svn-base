/**
 * @file main.test.c
 * @brief Tests pour le module principal.
 * @author kylian.adam@uha.fr
 */

#include "main.test.h"
#include "../src/main.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <sys/types.h>
#include <sys/wait.h>

// Déclaration des fonctions externes utilisées dans les tests
void force_quit_gtk(void);
int test_game_in_bounds(void);
int test_game_piece_owner(void);
int test_launch_exhaustive(void);
int launch(int argc, char *argv[]);

// Test validate_ip_port
int test_validate_ip_port(void) {
    int fails = 0;
    if (!validate_ip_port("127.0.0.1:11000")) { printf("validate_ip_port(\"127.0.0.1:11000\") devrait retourner 1\n"); fails++; }
    if (validate_ip_port("127.0.0.1"))       { printf("validate_ip_port(\"127.0.0.1\") devrait retourner 0\n"); fails++; }
    if (validate_ip_port("127.0.0.1:"))      { printf("validate_ip_port(\"127.0.0.1:\") devrait retourner 0\n"); fails++; }
    if (validate_ip_port(":11000"))           { printf("validate_ip_port(\":11000\") devrait retourner 0\n"); fails++; }
    if (validate_ip_port("abc.def:11000"))    { printf("validate_ip_port(\"abc.def:11000\") devrait retourner 0\n"); fails++; }
    if (!validate_ip_port("192.168.1.1:1"))  { printf("validate_ip_port(\"192.168.1.1:1\") devrait retourner 1\n"); fails++; }
    return fails;
}

// Helper pour fermer la fenêtre GTK après un délai
static void* close_gtk_after_delay(void* arg) {
    (void)arg;
    sleep(1);
    force_quit_gtk();
    return NULL;
}

// Test play_local (lance et ferme automatiquement)
int test_play_local(void) {
    int fails = 0;
    pthread_t closer;
    pthread_create(&closer, NULL, close_gtk_after_delay, NULL);
    int ret = play_local(0);
    if (ret != 0 && ret != 1) {
        printf("play_local(0) retourne %d (attendu 0 ou 1)\n", ret);
        fails++;
    }
    pthread_join(closer, NULL);
    return fails;
}

int test_play_network_ia(void) {
    int fails = 0;
    pid_t pid_server = fork();
    if (pid_server == 0) {
        // Processus fils : serveur IA via launch
        char *argv[] = {"prog", "-s", "11000", "-ia"};
        exit(launch(4, argv));
    }
    usleep(200000); // Laisse le serveur démarrer

    pid_t pid_client = fork();
    if (pid_client == 0) {
        // Processus fils : client IA via launch
        char *argv[] = {"prog", "-c", "127.0.0.1:11000", "-ia"};
        exit(launch(4, argv));
    }

    int status_server = 0, status_client = 0;
    waitpid(pid_server, &status_server, 0);
    waitpid(pid_client, &status_client, 0);

    int code_server = WEXITSTATUS(status_server);
    int code_client = WEXITSTATUS(status_client);

    if (code_server != 0 && code_server != 1) {
        printf("launch serveur (IA) retourne %d (attendu 0 ou 1)\n", code_server);
        fails++;
    }
    if (code_client != 0 && code_client != 1) {
        printf("launch client (IA) retourne %d (attendu 0 ou 1)\n", code_client);
        fails++;
    }
    return fails;
}

int test_launch_exhaustive(void) {
    int fails = 0;
    char *argv[6];

    // 1. Pas assez d'arguments
    int argc = 1;
    argv[0] = "prog";
    if (launch(argc, argv) != 1) { printf("fail: argc < 2\n"); fails++; }

    // 2. Mode local simple
    argc = 2;
    argv[1] = "-l";
    if (launch(argc, argv) != 0 && launch(argc, argv) != 1) { printf("fail: -l sans IA\n"); fails++; }

    // 3. Mode local avec IA
    argc = 3;
    argv[1] = "-l"; argv[2] = "-ia";
    if (launch(argc, argv) != 0 && launch(argc, argv) != 1) { printf("fail: -l -ia\n"); fails++; }

    // 4. Mode local avec argument invalide
    argc = 3;
    argv[1] = "-l"; argv[2] = "foo";
    if (launch(argc, argv) != 1) { printf("fail: -l foo\n"); fails++; }

    // 5. Mode local avec trop d'arguments
    argc = 4;
    argv[1] = "-l"; argv[2] = "-ia"; argv[3] = "foo";
    if (launch(argc, argv) != 1) { printf("fail: -l -ia foo\n"); fails++; }

    // 6. Mode serveur sans port
    argc = 2;
    argv[1] = "-s";
    if (launch(argc, argv) != 1) { printf("fail: -s sans port\n"); fails++; }

    // 7. Mode serveur avec port non numérique
    argc = 3;
    argv[1] = "-s"; argv[2] = "abc";
    if (launch(argc, argv) != 1) { printf("fail: -s port non numérique\n"); fails++; }

    // 8. Mode serveur avec port et argument invalide
    argc = 4;
    argv[1] = "-s"; argv[2] = "1234"; argv[3] = "foo";
    if (launch(argc, argv) != 1) { printf("fail: -s port foo\n"); fails++; }

    // 9. Mode serveur avec trop d'arguments
    argc = 5;
    argv[1] = "-s"; argv[2] = "1234"; argv[3] = "-ia"; argv[4] = "foo";
    if (launch(argc, argv) != 1) { printf("fail: -s port -ia foo\n"); fails++; }

    // 10. Mode client avec IP:PORT invalide
    argc = 3;
    argv[1] = "-c"; argv[2] = "127.0.0.1";
    if (launch(argc, argv) != 1) { printf("fail: -c IP:PORT invalide\n"); fails++; }

    // 11. Mode client sans IP:PORT
    argc = 2;
    argv[1] = "-c";
    if (launch(argc, argv) != 1) { printf("fail: -c sans IP:PORT\n"); fails++; }

    // 12. Mode client avec IP:PORT et argument invalide
    argc = 4;
    argv[1] = "-c"; argv[2] = "127.0.0.1:5678"; argv[3] = "foo";
    if (launch(argc, argv) != 1) { printf("fail: -c IP:PORT foo\n"); fails++; }

    // 13. Mode client avec trop d'arguments
    argc = 5;
    argv[1] = "-c"; argv[2] = "127.0.0.1:5678"; argv[3] = "-ia"; argv[4] = "foo";
    if (launch(argc, argv) != 1) { printf("fail: -c IP:PORT -ia foo\n"); fails++; }

    // 14. Argument initial inconnu
    argc = 2;
    argv[1] = "foo";
    if (launch(argc, argv) != 1) { printf("fail: argument initial inconnu\n"); fails++; }

    // 15. Mode serveur avec port valide
    argc = 3;
    argv[1] = "-s"; argv[2] = "12345";
    if (launch(argc, argv) != 0) { printf("fail: -s port valide\n"); fails++; }

    // 16. Mode serveur avec port valide et IA
    argc = 4;
    argv[1] = "-s"; argv[2] = "12345"; argv[3] = "-ia";
    if (launch(argc, argv) != 0) { printf("fail: -s port valide -ia\n"); fails++; }

    return fails;
}

int batch_tests_main() {
    printf("=== Test de la logique principale ===\n");
    int test_count = 0, failures = 0;

    failures += test_validate_ip_port(); test_count++;
    failures += test_play_network_ia();  test_count++;
    failures += test_launch_exhaustive(); test_count++;

    if (!test_game_in_bounds()) {
        failures++;
    }
    test_count++;

    if (!test_game_piece_owner()) {
        failures++;
    }
    test_count++;

    printf("%d failures out of %d tests in main.c\n", failures, test_count);
    printf("=== Fin des tests de la logique principale ===\n");
    return failures;
}