//
// Created by romel on 2024/11/23.
//

#ifndef MINIMAX_H
#define MINIMAX_H

#include "evaluate.h"

#define MAX_PATH_LENGTH 7
#define SEARCH_DEPTH 8
#define MAX_DEPTH 8
#define TIME_LIMIT 10000

typedef struct {
    int steps[MAX_PATH_LENGTH][2];
    int length;
} Path;

int iterative_deepening(Evaluate *eval, int alpha, int beta, int role, Path *path);
int minimax(Evaluate *eval, int depth, int alpha, int beta, int role, Path *path);

#endif //MINIMAX_H
