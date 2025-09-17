//
// Created by romel on 2024/11/20.
//

#ifndef COMMON_H
#define COMMON_H
// 棋盘尺寸
#define MAX_ROW 15
#define MAX_COL 15

// 棋子类型
#define WHITE -1
#define BLACK 1
#define RECENT_WHITE -2
#define RECENT_BLACK 2
#define BLANK 0
#define WALL 3

// 分数和评估常量
#define MAX_SCORE 1000000000
#define MIN_SCORE -1000000000

// AI考虑的最大走法数量
#define MAX_MOVES 400

typedef struct {
    int size;
    int board[MAX_ROW + 2][MAX_COL + 2];  // 棋盘，多加两圈用于表示墙壁
    int black_scores[MAX_ROW][MAX_COL];   // 黑棋在各点的得分
    int white_scores[MAX_ROW][MAX_COL];   // 白棋在各点的得分

    // 缓存机制，用于存储每个位置、每个方向、每个角色的棋型
    int shape_cache[MAX_ROW][MAX_COL][4][2];  // [x][y][direction][role_index]
} Evaluate;


#endif //COMMON_H
