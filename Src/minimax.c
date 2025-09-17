//
// Created by romel on 2024/11/23.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "minimax.h"
#include "evaluate.h"
#include "game.h"

// 判断是否为关键局面
static int is_critical_position(Evaluate *eval, int role) {
    int opponent = -role;
    
    for (int i = 0; i < eval->size; i++) {
        for (int j = 0; j < eval->size; j++) {
            if (eval->board[i + 1][j + 1] == BLANK) {
                // 检查我方威胁
                emplace_chess(eval, i, j, role);
                int my_score = evaluate_position(eval, i, j, role);
                remove_chess(eval, i, j);

                // 检查对手威胁
                emplace_chess(eval, i, j, opponent);
                int opp_score = evaluate_position(eval, i, j, opponent);
                remove_chess(eval, i, j);

                // 如果存在高威胁局面
                if (my_score >= THREE or opp_score >= THREE) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

// 迭代加深搜索
int iterative_deepening(Evaluate *eval, int alpha, int beta, int role, Path *path) {
    clock_t start = clock();
    int best_value = MIN_SCORE;
    Path best_path = {.length = 0};
    int current_depth = 4;  // Start from depth 4

    // Check if it's a critical position
    int critical = is_critical_position(eval, role);
    int max_search_depth = critical ? MAX_DEPTH : SEARCH_DEPTH;

    while (current_depth <= max_search_depth) {
        Path temp_path = {.length = 0};
        int value = minimax(eval, current_depth, alpha, beta, role, &temp_path);
        
        // Check time limit
        clock_t current = clock();
        if ((current - start) * 1000 / CLOCKS_PER_SEC >= TIME_LIMIT) {
            break;
        }

        // Update best path
        if (value > best_value) {
            best_value = value;
            best_path = temp_path;
        }

        // If found winning move, return immediately
        if (value >= FIVE) {
            break;
        }

        current_depth++;
    }

    // Copy the best path to output path
    if (path != NULL) {
        *path = best_path;
        // Print final decision
        int x = best_path.steps[0][0];
        int y = best_path.steps[0][1];
        char *shape_name;
        
        print_strategy = 1;  // 开启策略打印
        int score = evaluate_position(eval, x, y, role);
        print_strategy = 0;  // 关闭策略打印
        
        if (score >= FIVE) shape_name = "Five in a row";
        else if (score >= FOUR) shape_name = "Live Four";
        else if (score >= BLOCK_FOUR) shape_name = "Blocked Four";
        else if (score >= THREE) shape_name = "Live Three";
        else if (score >= BLOCK_THREE) shape_name = "Blocked Three";
        else if (score >= TWO) shape_name = "Live Two";
        else if (score >= BLOCK_TWO) shape_name = "Blocked Two";
        else shape_name = "Normal move";

        printf("\nAI places at %c%d (%s, score: %d)\n", 
               'A' + y, x + 1, shape_name, best_value);
    }

    return best_value;
}

int minimax(Evaluate *eval, int depth, int alpha, int beta, int role, Path *path) {
    // 检查是否达到搜索深度
    if (depth == 0) {
        return evaluate(eval, role);
    }

    int moves[MAX_MOVES][2];
    int move_count;
    get_moves(eval, role, moves, &move_count);

    if (move_count == 0) {
        return MIN_SCORE;
    }

    // 首先检查必胜着法
    for (int i = 0; i < move_count; i++) {
        int x = moves[i][0];
        int y = moves[i][1];
        
        emplace_chess(eval, x, y, role);
        int score = evaluate_position(eval, x, y, role);
        remove_chess(eval, x, y);
        
        if (score >= FIVE) {
            if (path != NULL) {
                path->steps[0][0] = x;
                path->steps[0][1] = y;
                path->length = 1;
            }
            return FIVE;
        }
    }

    // 对走法进行排序
    Move sorted_moves[MAX_MOVES];
    for (int i = 0; i < move_count; i++) {
        sorted_moves[i].x = moves[i][0];
        sorted_moves[i].y = moves[i][1];
        
        emplace_chess(eval, moves[i][0], moves[i][1], role);
        sorted_moves[i].score = evaluate_position(eval, moves[i][0], moves[i][1], role);
        remove_chess(eval, moves[i][0], moves[i][1]);
    }
    qsort(sorted_moves, move_count, sizeof(Move), compare_moves);

    int best_value = MIN_SCORE;
    int best_move_x = -1, best_move_y = -1;

    for (int i = 0; i < move_count; i++) {
        int x = sorted_moves[i].x;
        int y = sorted_moves[i].y;

        // 如果是黑棋，检查禁手
        if (role == BLACK) {
            int temp_board[MAX_ROW][MAX_COL] = {0};
            for (int i = 0; i < MAX_ROW; i++) {
                for (int j = 0; j < MAX_COL; j++) {
                    temp_board[i][j] = eval->board[i + 1][j + 1];
                }
            }
            if (is_forbidden(x, y, temp_board)) {
                continue;  // 跳过禁手位置
            }
        }

        emplace_chess(eval, x, y, role);
        Path new_path = {.length = 0};
        int value = -minimax(eval, depth - 1, -beta, -alpha, -role, &new_path);
        remove_chess(eval, x, y);

        if (value > best_value) {
            best_value = value;
            best_move_x = x;
            best_move_y = y;

            if (path != NULL) {
                path->steps[0][0] = best_move_x;
                path->steps[0][1] = best_move_y;
                path->length = 1;
                for (int j = 0; j < new_path.length and path->length < MAX_PATH_LENGTH - 1; j++) {
                    path->steps[path->length][0] = new_path.steps[j][0];
                    path->steps[path->length][1] = new_path.steps[j][1];
                    path->length++;
                }
            }
        }

        alpha = (alpha > value) ? alpha : value;
        if (alpha >= beta) {
            break;
        }
    }

    return best_value;
}

