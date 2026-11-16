/** 
 * @file test-principal.c
 * @brief Point d'entrée en cas de test
 * @author baptiste.garnier@uha.fr
 */
#include <stdio.h>

//#include "ai.test.h"
#include "game.test.h"
#include "com.test.h"
#include "main.test.h"
//#include "ui.test.h"


unsigned short erreurs;
int main(){
    erreurs = erreurs + batch_tests_game() ;
    if (batch_tests_com()) erreurs++; // car tout fonctionne ou rien
    erreurs = erreurs + batch_tests_main() ;
    printf("Nombre total d'erreurs: %d\n", erreurs);
}
