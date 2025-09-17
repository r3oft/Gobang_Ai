//
// Created by romel on 2024/10/11.
//

#ifndef EVALUATE_H
#define EVALUATE_H


#include "common.h"
#include <string.h>

// 添加在文件开头的全局变量声明部分
extern int print_strategy;  // 控制是否打印策略分析

// 评分表 - 根据具体棋型定义分值
#define FIVE         9999    // 五连 ●●●●●
#define FOUR        5000    // 活四系列(四颗连子)
                           // 包括: ●●●●、●●●x●x●●●(长连)
                           // ●●x●●x●●、●x●●●x●

#define BLOCK_FOUR   1000    // 冲四系列
                           // 包括: ○●●●●(单冲四)
                           // ●●●x●、●●x●●(双冲四)

#define THREE        800    // 活三系列
                           // 包括: ●●●(活三)
                           // ●●x●(跳活三)
                           // ●x●x●x●(多跳活三)
                           // ●●x●xx●●(复合活三)
                           // ●●●xxx●●●(复合活三)

#define BLOCK_THREE  100    // 眠三系列

#define TWO          45     // 活二系列
                           // 包括: ●●(基本活二)
                           // ●x●(跳活二)
                           // ○●●●(被封活二)
                           // ○x●●●x○(跳二)
                           // ○●●x●、○●x●●(半封活二)
                           // ○●x●x●、○●xx●●(复合活二)

#define BLOCK_TWO    20     // 眠二系列
                           // 包括: ●xx●(断二)
                           // ○●●(基本眠二)
                           // ○●x●、○●xx●(复合眠二)

#define ONE          10     // 单子 ●
#define BLOCK_ONE    10     // 眠一

// 防守系数 - 用于调整防守权重
#define DEFENSE_FACTOR 1.6  // 略微提高防守权重

// 连续棋子的奖励系数
#define CONTINUOUS_FACTOR 2  // 连续棋子的倍数奖励

// 棋型判定标识
#define SHAPE_NONE 0        // 无棋型
#define SHAPE_ONE 1         // 单子
#define SHAPE_BLOCK_ONE 10  // 眠一
#define SHAPE_BLOCK_TWO 20  // 眠二
#define SHAPE_TWO 2         // 活二
#define SHAPE_BLOCK_THREE 30 // 眠三
#define SHAPE_THREE 3       // 活三
#define SHAPE_BLOCK_FOUR 40 // 冲四
#define SHAPE_FOUR 4        // 活四
#define SHAPE_FIVE 5        // 五连

typedef struct Move {
    int x;
    int y;
    int score;
} Move;

// 初始化评估器
void init_evaluate(Evaluate *eval);
// 在评估器中模拟落子（内部使用）
void emplace_chess(Evaluate *eval, int x, int y, int role);
// 在评估器中撤销落子（内部使用）
void remove_chess(Evaluate *eval, int x, int y);
// 评估当前棋盘得分
int evaluate(Evaluate *eval, int role);
// 获取所有可行的走法（供 AI 算法使用）
void get_moves(Evaluate *eval, int role, int moves[][2], int *move_count);

void update_point(Evaluate *eval, int x, int y);
void update_single_point(Evaluate *eval, int x, int y, int role);

void get_line(Evaluate *eval, int x, int y, int dir_x, int dir_y, int line[]);

int get_shape(int line[], int role);
void evaluate_point(Evaluate *eval, int x, int y, int role);

// 更新缓存
void update_cache(Evaluate *eval, int x, int y);
void make_move(Evaluate *eval, int x, int y, int role);

int compare_moves(const void *a, const void *b);

// 添加新函数声明
int evaluate_position(Evaluate *eval, int x, int y, int role);

#endif //EVALUATE_H
