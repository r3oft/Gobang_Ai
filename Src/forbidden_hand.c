//
// Created by romel on 2024/9/28.
//
// forbidden_hand.c

#include "game.h"

// 定义方向增量：0~7，右开始，逆时针旋转
int row_sign[8] = {0, -1, -1, -1, 0, 1, 1, 1};
int col_sign[8] = {1, 1, 0, -1, -1, -1, 0, 1};

// 判断坐标是否在棋盘范围内
int judge_in_bounds(int row, int col) {
    return (row >= 0 and row < MAX_ROW and col >= 0 and col < MAX_COL);
}

// 判断是否存在活三的函数
int is_double_three(int nearby1, int nearby2, int side1, int side2, int jump1, int jump2,
            int row, int col, int sign, int chessboard[MAX_ROW][MAX_COL]) {
    int judge_going_4 = 0;
    int middle = nearby1 + nearby2 + 1;

    if ((middle + side1) == 3 and jump1) {
        row += (nearby1 + 1) * row_sign[sign];
        col += (nearby1 + 1) * col_sign[sign];
        if (judge_in_bounds(row, col)) {
            chessboard[row][col] = BLACK;
            judge_going_4 += is_forbidden_new(row, col, chessboard);
            chessboard[row][col] = BLANK;
        }
    }
    else if ((middle + side2) == 3 and jump2) {
        row -= (nearby2 + 1) * row_sign[sign];
        col -= (nearby2 + 1) * col_sign[sign];
        if (judge_in_bounds(row, col)) {
            chessboard[row][col] = BLACK;
            judge_going_4 += is_forbidden_new(row, col, chessboard);
            chessboard[row][col] = BLANK;
        }
    }
    return judge_going_4;
}

// 判断是否存在四连
int is_double_four(int side1, int side2, int jump1, int jump2, int middle) {
    if (middle == 4) {
        if (jump1 and jump2) {
            if (side1 and side2)
                return 0;
            if (side1 or side2)
                return 1;
            return 4;
        }
        else if (jump1 or jump2) {
            if (side1 or side2)
                return 0;
            return 1;
        }
        return 0;
    }
    if ((middle + side1 == 4) or (middle + side2 == 4)) {
        if (side1 == side2)
            return 2;
        return 1;
    }
    return 0;
}

// 判断是否为禁手
int is_forbidden(int row_place, int col_place, int chessboard[MAX_ROW][MAX_COL]) {
    int row, col;
    int nearby[8], jump[8], neighbor[8], end[8];
    int middle[4];

    for (int i = 0; i < 8; i++) {
        row = row_place;
        col = col_place;
        nearby[i] = jump[i] = neighbor[i] = end[i] = 0;

        row += row_sign[i];
        col += col_sign[i];

        while (judge_in_bounds(row, col) and jump[i] <= 1) {
            int cell = chessboard[row][col];
            if (cell == BLANK) {
                jump[i]++;
            }
            else if (cell == BLACK or cell == RECENT_BLACK) {
                if (jump[i] == 0)
                    nearby[i]++;
                else
                    neighbor[i]++;
            }
            else {
                break;
            }
            row += row_sign[i];
            col += col_sign[i];
        }
        end[i] = judge_in_bounds(row, col) ? chessboard[row][col] : WHITE;
    }

    for (int i = 0; i < 4; i++)
        middle[i] = nearby[i] + nearby[i + 4] + 1;

    int special_3 = 0;
    int special_4 = 0;
    int special_long = 0;
    int special_4_temp;

    for (int i = 0; i < 4; i++) {
        if (middle[i] == 5)
            return 0;
        else if (middle[i] > 5)
            ++special_long;
        if (special_long == 0) {
            special_4_temp = is_double_four(neighbor[i], neighbor[i + 4], jump[i], jump[i + 4], middle[i]);
            if (special_4_temp)
                special_4 += special_4_temp % 3;
            else
                special_3 += is_double_three(nearby[i], nearby[i + 4], neighbor[i], neighbor[i + 4],
                                     jump[i], jump[i + 4], row_place, col_place, i, chessboard);
        }
    }
    if (special_long or special_4 >= 2 or special_3 >= 2)
        return 1; // 禁手
    return 0;     // 合法
}

// 判断禁手（用于递归）
int is_forbidden_new(int row_place, int col_place, int chessboard[MAX_ROW][MAX_COL]) {
    int row, col;
    int nearby[8], jump[8], neighbor[8], end[8];
    int middle[4];

    for (int i = 0; i < 8; i++) {
        row = row_place;
        col = col_place;
        nearby[i] = jump[i] = neighbor[i] = end[i] = 0;

        row += row_sign[i];
        col += col_sign[i];

        while (judge_in_bounds(row, col) and jump[i] <= 1) {
            int cell = chessboard[row][col];
            if (cell == BLANK) {
                jump[i]++;
            }
            else if (cell == BLACK or cell == RECENT_BLACK) {
                if (jump[i] == 0)
                    nearby[i]++;
                else
                    neighbor[i]++;
            }
            else {
                break;
            }
            row += row_sign[i];
            col += col_sign[i];
        }
        end[i] = judge_in_bounds(row, col) ? chessboard[row][col] : WHITE;
    }

    for (int i = 0; i < 4; i++)
        middle[i] = nearby[i] + nearby[i + 4] + 1;

    int special_3 = 0;
    int special_4 = 0;
    int special_long = 0;
    int special_4_temp;
    int live_4_exist = 0;

    for (int i = 0; i < 4; i++) {
        if (middle[i] == 5)
            return 0;
        else if (middle[i] > 5)
            ++special_long;
        if (special_long == 0) {
            special_4_temp = is_double_four(neighbor[i], neighbor[i + 4], jump[i], jump[i + 4], middle[i]);
            if (special_4_temp == 4)
                live_4_exist = 1;
            if (special_4_temp)
                special_4 += special_4_temp % 3;
            else
                special_3 += is_double_three(nearby[i], nearby[i + 4], neighbor[i], neighbor[i + 4],
                                     jump[i], jump[i + 4], row_place, col_place, i, chessboard);
        }
    }
    if (special_long or special_4 >= 2 or special_3 >= 2)
        return 0;
    if (live_4_exist)
        return 1;
    return 0;
}