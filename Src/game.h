//
// Created by romel on 2024/9/28.
//

#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>

#include "common.h"

// Player roles for human vs AI mode
extern int AI_ROLE;
extern int HUMAN_ROLE;
extern Evaluate evaluator;

void draw_chessboard(int row, int col, int chessboard[][MAX_COL]);
void draw_chessman(int type, char *tableline);

void menu(void);

void person_vs_person(void);
void person_vs_ai(void);

int is_full(int chessboard[][MAX_COL], int row, int col);
int is_win(int chessboard[][MAX_COL]);

int judge_in_bounds(int row, int col);
int is_forbidden(int row_place, int col_place, int chessboard[][MAX_COL]);
int is_forbidden_new(int row_place, int col_place, int chessboard[MAX_ROW][MAX_COL]);
int is_double_three(int nearby1, int nearby2, int side1, int side2, int jump1, int jump2,
            int row, int col, int sign, int chessboard[][MAX_COL]);
int is_double_four(int side1, int side2, int jump1, int jump2, int middle);

void select_player_color(void);

#endif //GAME_H
