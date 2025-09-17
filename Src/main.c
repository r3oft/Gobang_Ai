//
// Created by romel on 2024/9/28.
//
#include <time.h>

#include "game.h"
#include "minimax.h"
#include "hash.h"

int main(void) {
    srand((unsigned int)time(NULL));
    int choice;
    menu();
    while (1) {
        scanf("%d", &choice);
        switch (choice) {
            case 1:
                person_vs_person();
                break;
            case 2:
                init_hash_table();
                person_vs_ai();
                break;
            case 3:
                exit(0);
                break;
            default:
                printf("Input error, please choose again.\n");
        }
    }

    return 0;
}

