/** 
 * @file game.test.c
 * @brief Tests pour game.c, point d'entrée du système
 * @author baptiste.garnier@uha.fr
 */

#include <stdio.h>
#include <stdbool.h>
#include "../src/game.h"

bool test_game_in_bounds(){
    for (short r = 0; r < BOARD_SIZE; r++) { //rows
        for (short c =0; c < BOARD_SIZE; c++) { //columns
            if (! game_in_bounds(r, c)) {
                printf("La case a la ligne %d et à la colomne %d est pourtant valide",r,c);
                return false;
            }
        }
    }
    int bad_rows[]={-1,-5,BOARD_SIZE+1};
    int bad_columns[]={-1,-5,BOARD_SIZE+1};
    for (int i = 0; bad_rows[i] < BOARD_SIZE; i++) {
        for (int j =0; bad_columns[j] < BOARD_SIZE; j++) {
            if (game_in_bounds(bad_rows[i], bad_columns[j])) {
                printf("La case a la ligne %d et à la colomne %d n'est pas dans\
                    le plateau et est pourtant acceptée",bad_rows[i], bad_columns[j]);
                return false;
            }
        }
    }
    return true;
}

bool test_game_piece_owner(){
    const Piece pieces[5]={ PIECE_NONE,PIECE_SOLDIER_BLUE,PIECE_KING_BLUE,
                          PIECE_SOLDIER_RED,PIECE_KING_RED};
    const Player expected[5] = { PLAYER_NONE, PLAYER_BLUE, PLAYER_BLUE, PLAYER_RED, PLAYER_RED };
    for (size_t i = 0; i < 5; i++) {
        Player got = game_piece_owner(pieces[i]);
        if (got != expected[i]) {
            printf("game_piece_owner(%d) devrait retourner %d mais retourne %d\n",
                pieces[i], expected[i], got);
            return false;
        }
    }
    return true;
}

unsigned int batch_tests_game() {
    printf("Test de game\n");
    int test_count = 0, failures = 0;
    if (!test_game_in_bounds()) {
        failures++;
    }
    test_count++;

    if (!test_game_piece_owner()) {
        failures++;
    }
    test_count++;

    printf("%d failures out of %d tests in game.c\n", failures, test_count);
    return failures;
}
