//
// Created by romel on 2024/10/11.
//

#include "evaluate.h"
#include "game.h"
#include "hash.h"

// 函数声明
static int count_continuous(int line[], int role);
static inline int max(int a, int b);  // 添加 max 函数声明

// 实现 max 函数
static inline int max(int a, int b) {
    return (a > b) ? a : b;
}

unsigned long long current_board_hash = 0;

int print_strategy = 0;  // 默认不打印策略

void print_board(Evaluate *eval) {
    for (int i = 0; i < eval->size + 2; i++) {
        for (int j = 0; j < eval->size + 2; j++) {
            printf("%d ", eval->board[i][j]);
        }
        printf("\n");
    }
}

void init_evaluate(Evaluate *eval) {
    eval->size = MAX_ROW;  // 假设 MAX_ROW == MAX_COL
    // 初始化棋盘，设置墙壁和空位
    for (int i = 0; i < eval->size + 2; i++) {
        for (int j = 0; j < eval->size + 2; j++) {
            if (i == 0 or j == 0 or i == eval->size + 1 or j == eval->size + 1) {
                eval->board[i][j] = WALL;
            }
            else {
                eval->board[i][j] = BLANK;
            }
        }
    }

    // 初始化得分
    memset(eval->black_scores, 0, sizeof(eval->black_scores));
    memset(eval->white_scores, 0, sizeof(eval->white_scores));
    memset(eval->shape_cache, -1, sizeof(eval->shape_cache));
    current_board_hash = compute_board_hash(eval);
}

// 模拟在评器中落子（内部使用）
void emplace_chess(Evaluate *eval, int x, int y, int role) {
    eval->board[x + 1][y + 1] = role;  // 棋盘多了两圈墙壁，坐标需要加 1
    update_point(eval, x, y);
    current_board_hash = compute_board_hash(eval);
}

// 模拟在评估器中撤销落子（内部使用）
void remove_chess(Evaluate *eval, int x, int y) {
    eval->board[x + 1][y + 1] = BLANK;  // 设置为空位
    update_point(eval, x, y);
    current_board_hash = compute_board_hash(eval);
}

void update_point(Evaluate *eval, int x, int y) {
    update_single_point(eval, x, y, BLACK);
    update_single_point(eval, x, y, WHITE);
    // 更新周围的点
    int directions[4][2] = {
        {0, 1},  // 水平
        {1, 0},  // 垂直
        {1, 1},  // 正斜
        {1, -1}  // 反斜
    };
    for (int i = 0; i < 4; i++) {
        int dx = directions[i][0];
        int dy = directions[i][1];
        for (int step = -4; step <= 4; step++) {
            if (step == 0) continue;
            int nx = x + dx * step;
            int ny = y + dy * step;
            if (nx >= 0 and nx < eval->size and ny >= 0 and ny < eval->size and eval->board[nx + 1][ny + 1] == BLANK) {
                update_single_point(eval, nx, ny, BLACK);
                update_single_point(eval, nx, ny, WHITE);
            }
        }
    }
}

void update_single_point(Evaluate *eval, int x, int y, int role) {
    evaluate_point(eval, x, y, role);
}

void get_line(Evaluate *eval, int x, int y, int dir_x, int dir_y, int line[]) {
    line[4] = eval->board[x + 1][y + 1];
    // 向前方向
    for (int i = 1; i <= 4; i++) {
        int nx = x + dir_x * i ;
        int ny = y + dir_y * i ;
        int idx = 4 + i;

        if (nx >= 0 and nx < eval->size and ny >= 0 and ny < eval->size)
            line[idx] = eval->board[nx + 1][ny + 1];
        else
            line[idx] = WALL;
    }
    // 向后方向
    for (int i = 1; i <= 4; i++) {
        int nx = x - dir_x * i;
        int ny = y - dir_y * i;
        int idx  =4 - i;

        if (nx >= 0 and nx < eval->size and ny >= 0 and ny < eval->size)
            line[idx] = eval->board[nx + 1][ny + 1];
        else
            line[idx] = WALL;
    }
}


int evaluate(Evaluate *eval, int role) {
    int my_score = 0;
    int opponent_score = 0;
    int opponent = -role;

    // 遍历整个棋盘
    for (int i = 0; i < eval->size; i++) {
        for (int j = 0; j < eval->size; j++) {
            int piece = eval->board[i + 1][j + 1];
            
            // 评估已有棋子的局面
            if (piece == role) {
                my_score += evaluate_position(eval, i, j, role);
            } 
            else if (piece == opponent) {
                opponent_score += evaluate_position(eval, i, j, opponent);
            }
            // 评估空位的潜力
            else if (piece == BLANK) {
                if (role == BLACK) {
                    my_score += eval->black_scores[i][j];
                    opponent_score += eval->white_scores[i][j];
                } else {
                    my_score += eval->white_scores[i][j];
                    opponent_score += eval->black_scores[i][j];
                }
            }
        }
    }

    // 根据角色调整评分权重
    if (role == BLACK) {
        return my_score - opponent_score * DEFENSE_FACTOR;
    } else {
        return my_score * DEFENSE_FACTOR - opponent_score;
    }
}

// 在 evaluate.c 中添加策略分析函数
static const char* analyze_strategy(int attack_score, int defense_score) {
    // 首先判断是否有必胜/必防机会
    if (attack_score >= FIVE) {
        return "Attacking: Making winning move";
    } else if (defense_score >= FIVE) {
        return "Defending: Blocking opponent's winning move";
    }

    // 判断攻守比例
    double attack_ratio = attack_score / (double)(defense_score + 1);
    double defense_ratio = defense_score / (double)(attack_score + 1);

    // 调整判断阈值
    if (attack_score >= THREE * 2) {  // 有双三或更强的进攻
        if (defense_score >= FOUR) {
            return "Defending: Critical defense needed";
        }
        return "Attacking: Strong offensive position";
    } 
    
    if (defense_score >= THREE) {  // 对手有活三或更强的威胁
        return "Defending: Blocking opponent's threat";
    }

    // 根据攻守比例判断策略
    if (attack_ratio > 2.0) {
        return "Attacking: Pressing advantage";
    } else if (defense_ratio > 2.0) {
        return "Defending: Under pressure";
    } else if (attack_ratio > 1.2) {
        return "Attacking: Building position";
    } else if (defense_ratio > 1.2) {
        return "Defending: Countering opponent";
    }

    // 如果分数都很低，说明是在布局
    if (attack_score < TWO and defense_score < TWO) {
        return "Neutral: Early game positioning";
    }

    return "Neutral: Balanced position";
}

/*
// 修改函数返回类型和参数
static int check_consecutive_pieces(Evaluate *eval, int role, int *best_x, int *best_y) {
    int max_consecutive = 0;
    int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
    
    // 遍历整个棋盘
    for (int i = 0; i < eval->size; i++) {
        for (int j = 0; j < eval->size; j++) {
            // 检查每个方向
            for (int d = 0; d < 4; d++) {
                int dx = directions[d][0];
                int dy = directions[d][1];
                
                // 如果当前位置是空的，检查它是否是一个潜在的威胁位置
                if (eval->board[i + 1][j + 1] == BLANK) {
                    int left_count = 0;
                    int right_count = 0;
                    
                    // 向左检查连续棋子
                    int left_x = i - dx;
                    int left_y = j - dy;
                    while (left_x >= 0 and left_x < eval->size and
                           left_y >= 0 and left_y < eval->size and
                           eval->board[left_x + 1][left_y + 1] == role) {
                        left_count++;
                        left_x -= dx;
                        left_y -= dy;
                    }
                    
                    // 向右检查连续棋子
                    int right_x = i + dx;
                    int right_y = j + dy;
                    while (right_x >= 0 and right_x < eval->size and
                           right_y >= 0 and right_y < eval->size and
                           eval->board[right_x + 1][right_y + 1] == role) {
                        right_count++;
                        right_x += dx;
                        right_y += dy;
                    }
                    
                    // 如果空位两侧的连续棋子总数达到4个或以上，这是一个需要立即防守的位置
                    int total_connected = left_count + right_count;
                    if (total_connected >= 4) {
                        *best_x = i;
                        *best_y = j;
                        return total_connected;  // 立即返回，这是最高优先级的防守位置
                    }
                    // 更新最大连续数
                    if (total_connected > max_consecutive) {
                        max_consecutive = total_connected;
                        *best_x = i;
                        *best_y = j;
                    }
                }
            }
        }
    }
    return max_consecutive;
}
*/

static int check_consecutive_pieces(Evaluate *eval, int role, int *best_x, int *best_y) {
    int max_priority = 0;
    int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};

    for (int i = 0; i < eval->size; i++) {
        for (int j = 0; j < eval->size; j++) {
            if (eval->board[i + 1][j + 1] == BLANK) {
                for (int d = 0; d < 4; d++) {
                    int count = 1;
                    int blocks = 0;

                    // Check in both directions
                    for (int dir = -1; dir <= 1; dir += 2) {
                        int dx = directions[d][0] * dir;
                        int dy = directions[d][1] * dir;
                        int x = i + dx;
                        int y = j + dy;

                        while (x >= 0 && x < eval->size && y >= 0 && y < eval->size) {
                            if (eval->board[x + 1][y + 1] == role) {
                                count++;
                            }
                            else if (eval->board[x + 1][y + 1] == -role) {
                                blocks++;
                                break;
                            }
                            else {
                                break;
                            }
                            x += dx;
                            y += dy;
                        }
                    }

                    // Assign priority based on count and blocks
                    int priority = 0;
                    if (count == 4 and blocks == 0) {
                        priority = 100; // Open four (must block)
                    }
                    else if (count == 3 and blocks < 2) {
                        priority = 80; // Live three
                    }
                    else if (count == 2 and blocks < 2) {
                        priority = 60; // Live two
                    }

                    if (priority > max_priority) {
                        max_priority = priority;
                        *best_x = i;
                        *best_y = j;
                    }
                }
            }
        }
    }
    return max_priority;
}


// 修改评估函数
int evaluate_position(Evaluate *eval, int x, int y, int role) {
    // 如果是黑棋，需要检查禁手
    if (role == BLACK) {
        // 将评估器的棋盘状态转换为游戏棋盘状态进行禁手判断
        int temp_board[MAX_ROW][MAX_COL] = {0};
        for (int i = 0; i < MAX_ROW; i++) {
            for (int j = 0; j < MAX_COL; j++) {
                temp_board[i][j] = eval->board[i + 1][j + 1];
            }
        }
        
        // 如果是禁手位置，返回极低分数
        if (is_forbidden(x, y, temp_board)) {
            return MIN_SCORE;
        }
    }

    int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
    int total_score = 0;
    int line[9];
    int opponent = -role;

    int my_attack_score = 0;
    int my_defense_score = 0;

    // 1. 检查对手的连续棋子数
    int best_defense_x = -1, best_defense_y = -1;
    int opponent_consecutive = check_consecutive_pieces(eval, opponent, &best_defense_x, &best_defense_y);
    
    // 如果对手有连续棋子，这是高优先级防守位置
    if (opponent_consecutive >= 4) {
        // 如果当前位置就是最佳防守位置
        if (x == best_defense_x and y == best_defense_y) {
            return FIVE * 2;  // 最高优先级防守
        }
        // 如果不是最佳防守位置，给予极低的分数
        return -FIVE * 2;  // 惩罚不防守的选择
    } else if (opponent_consecutive >= 3) {
        // 如果当前位置就是最佳防守位置
        if (x == best_defense_x and y == best_defense_y) {
            return FIVE;  // 高优先级防守
        }
        // 如果不是最佳防守位置，给予较低的分数
        return -FIVE;
    }

    // 2. 检查是否能成五
    for (int i = 0; i < 4; i++) {
        get_line(eval, x, y, directions[i][0], directions[i][1], line);
        if (get_shape(line, role) == SHAPE_FIVE) {
            return FIVE * 2;
        }
    }

    // 3. 检查对手是否能成五
    for (int i = 0; i < 4; i++) {
        get_line(eval, x, y, directions[i][0], directions[i][1], line);
        if (get_shape(line, opponent) == SHAPE_FIVE) {
            return FIVE;
        }
    }

    // 4.算杀
    int kill_score = 0;
    int threat_count = 0;

    for (int i = 0; i < 4; i++) {
        get_line(eval, x, y, directions[i][0], directions[i][1], line);
        int shape = get_shape(line, role);
        if (shape == SHAPE_FOUR or shape == SHAPE_BLOCK_FOUR) {
            kill_score += FOUR * 2;
            threat_count++;
        } else if (shape == SHAPE_THREE) {
            kill_score += THREE * 2;
            threat_count++;
        }
    }
    if (threat_count >= 2)
        kill_score *= 2;
    if (kill_score > total_score)
        return kill_score;


    // 5. 评估每个方向
    for (int i = 0; i < 4; i++) {
        get_line(eval, x, y, directions[i][0], directions[i][1], line);
        
        // 我方进攻分数
        int my_shape = get_shape(line, role);
        switch (my_shape) {
            case SHAPE_FOUR: 
                my_attack_score += FOUR * 2; break;  // 活四权重加倍
            case SHAPE_BLOCK_FOUR: 
                my_attack_score += BLOCK_FOUR * 1.5; break;
            case SHAPE_THREE: 
                my_attack_score += THREE * 2; break;  // 活三很重要
            case SHAPE_BLOCK_THREE: 
                my_attack_score += BLOCK_THREE; break;
            case SHAPE_TWO: 
                my_attack_score += TWO; break;
        }

        // 对手在此位置的进攻分（用于防守判断）
        get_line(eval, x, y, directions[i][0], directions[i][1], line);
        int opp_shape = get_shape(line, opponent);
        switch (opp_shape) {
            case SHAPE_FOUR: 
                my_defense_score += FOUR * 2; break;  // 必须防守对手的活四
            case SHAPE_BLOCK_FOUR: 
                my_defense_score += BLOCK_FOUR * 1.5; break;
            case SHAPE_THREE: 
                my_defense_score += THREE * 1.5; break;
            case SHAPE_BLOCK_THREE: 
                my_defense_score += BLOCK_THREE; break;
        }


        // 连续棋子的额外奖励
        int continuous = count_continuous(line, role);
        if (continuous > 1) {
            my_attack_score += continuous * CONTINUOUS_FACTOR * 100;
        }
    }

    // 6. 考虑位置价值
    int center_x = eval->size / 2;
    int center_y = eval->size / 2;
    int distance = abs(x - center_x) + abs(y - center_y);
    double position_weight = 1.0 - (distance / (double)(eval->size * 2));

    // 7. 组合所有分数
    // 进攻分数
    total_score += my_attack_score * (1.0 + position_weight);
    
    // 防守分数（根据对手威胁程度调整权重）
    if (my_defense_score >= FOUR) {
        // 对手有活四，必须防守
        total_score = my_defense_score * 1.2;
    } else if (my_defense_score >= THREE) {
        // 对手有活三，需要重点防守
        total_score = (total_score > my_defense_score * 1.1) ? 
                      total_score : my_defense_score * 1.1;
    } else {
        // 正常防守权重
        total_score = (total_score > my_defense_score) ? 
                      total_score : my_defense_score;
    }

    // 8. 特殊情况处理
    // 如果这步棋既能防守又能进攻，给予额外奖励
    if (my_attack_score > 0 and my_defense_score > 0) {
        total_score += 1000;  // 双向作用的棋子更有价值
    }

    // 9. 检查是否能形成活三活四等威胁局面
    // int threat_count = 0;
    for (int i = 0; i < 4; i++) {
        get_line(eval, x, y, directions[i][0], directions[i][1], line);
        int shape = get_shape(line, role);
        if (shape == SHAPE_THREE or shape == SHAPE_FOUR) {
            threat_count++;
        }
    }
    if (threat_count >= 2) {
        total_score *= 2;  // 多个威胁给予额外奖励
    }

    // 在返回分数前分析并打印策略
    if (print_strategy) {
        const char* strategy;
        if (opponent_consecutive >= 3) {
            strategy = "Defending: Blocking opponent's consecutive pieces";
        } else {
            strategy = analyze_strategy(my_attack_score, my_defense_score);
        }
        printf("Strategy: %s (Opponent's consecutive pieces: %d)\n", 
               strategy, opponent_consecutive);
    }

    return total_score;
}


// 计算连续棋子数
static int count_continuous(int line[], int role) {
    int count = 0;
    int max_count = 0;
    
    for (int i = 0; i < 9; i++) {
        if (line[i] == role) {
            count++;
            if (count > max_count) {
                max_count = count;
            }
        } else {
            count = 0;
        }
    }
    
    return max_count;
}

/* Get shape type of a line */
int get_shape(int line[], int role) {
    int count = 0;      // 连续棋子数
    int block = 0;      // 封闭数
    int empty = 0;      // 空位数
    int len = 9;        // 线长度
    int empty_pos = -1; // 记录空位的位置
    int segments[3] = {0, 0, 0}; // 记录分段的连续棋子数
    int seg_count = 0;  // 分段数
    
    // 统计中心位置的连续棋子
    int center = 4;     // 中心位置
    int left = center - 1;  // 左扫描位置
    int right = center + 1; // 向右扫描位置
    
    // 先计算中心位��的连续棋子
    if (line[center] == role) {
        count = 1;
    } else {
        return SHAPE_NONE;  // 中心位置不是己方棋子，直接返回
    }
    
    // 向左扫描
    int left_count = 0;
    while (left >= 0) {
        if (line[left] == role) {
            count++;
            left_count++;
        } else if (line[left] == BLANK) {
            empty++;
            empty_pos = left;
            if (left_count > 0) {
                segments[seg_count++] = left_count;
            }
            left_count = 0;
            break;
        } else {
            block++;
            if (left_count > 0) {
                segments[seg_count++] = left_count;
            }
            break;
        }
        left--;
    }
    
    // 向右扫描
    int right_count = 0;
    while (right < len) {
        if (line[right] == role) {
            count++;
            right_count++;
        } else if (line[right] == BLANK) {
            empty++;
            if (empty_pos == -1) empty_pos = right;
            if (right_count > 0) {
                segments[seg_count++] = right_count;
            }
            right_count = 0;
            break;
        } else {
            block++;
            if (right_count > 0) {
                segments[seg_count++] = right_count;
            }
            break;
        }
        right++;
    }

    // 检查空位两侧的连续棋子
    if (empty == 1) {
        int total_connected = 0;
        for (int i = 0; i < seg_count; i++) {
            total_connected += segments[i];
        }
        total_connected++; // 加上中心位置的棋子

        // 如果空位两侧的连续棋子总数达到4个，这是一个需要立即防守的威胁
        if (total_connected >= 4) {
            return SHAPE_BLOCK_FOUR; // 将这种情况视为冲四
        }
    }

    // 原有的棋型判断逻辑
    if (count >= 5) return SHAPE_FIVE;
    
    if (count == 4) {
        if (block == 0) return SHAPE_FOUR;        // 活四
        if (block == 1) return SHAPE_BLOCK_FOUR;  // 冲四
        return SHAPE_NONE;
    }
    
    if (count == 3) {
        if (block == 0 and empty >= 2) return SHAPE_THREE;    // 活三
        if (block == 1 and empty >= 1) return SHAPE_BLOCK_THREE;  // 眠三
        return SHAPE_NONE;
    }
    
    if (count == 2) {
        if (block == 0 and empty >= 2) return SHAPE_TWO;      // 活二
        if (block == 1 and empty >= 1) return SHAPE_BLOCK_TWO;  // 眠二
        return SHAPE_NONE;
    }
    
    if (count == 1) {
        if (block == 0 and empty >= 2) return SHAPE_ONE;      // 活一
        if (block == 1 and empty >= 1) return SHAPE_BLOCK_ONE;  // 眠一
        return SHAPE_NONE;
    }
    
    return SHAPE_NONE;
}

void get_moves(Evaluate *eval, int role, int moves[][2], int *move_count) {
    *move_count = 0;
    Move temp_moves[MAX_ROW * MAX_COL];
    int count = 0;

    // 首先检查是否需要防守
    int best_defense_x = -1, best_defense_y = -1;
    int opponent = -role;
    int opponent_consecutive = check_consecutive_pieces(eval, opponent, &best_defense_x, &best_defense_y);

    // 如果发现需要防守的位置，直接返回防��位置
    if (opponent_consecutive >= 3 and best_defense_x >= 0 and best_defense_y >= 0) {
        moves[0][0] = best_defense_x;
        moves[0][1] = best_defense_y;
        *move_count = 1;
        if (print_strategy) {
            printf("Strategy: Critical defense needed at %d,%d (consecutive pieces: %d)\n", 
                   best_defense_x, best_defense_y, opponent_consecutive);
        }
        return;
    }

    // 记录所有对手棋子的位置
    int opponent_positions[MAX_ROW * MAX_COL][2];
    int opponent_count = 0;

    // 找出所有对手的棋子
    for (int i = 0; i < eval->size; i++) {
        for (int j = 0; j < eval->size; j++) {
            if (eval->board[i + 1][j + 1] == opponent) {
                opponent_positions[opponent_count][0] = i;
                opponent_positions[opponent_count][1] = j;
                opponent_count++;
            }
        }
    }

    // 如果找到了对手的棋子，检查它们周围的空位
    if (opponent_count > 0) {
        // 检查每个对手棋子周围2格范围内的空位
        for (int k = 0; k < opponent_count; k++) {
            int center_x = opponent_positions[k][0];
            int center_y = opponent_positions[k][1];
            
            // 检查周围2格范围内的所有位置
            for (int i = -2; i <= 2; i++) {
                for (int j = -2; j <= 2; j++) {
                    int x = center_x + i;
                    int y = center_y + j;
                    
                    // 检查位置是否有效且为空
                    if (x >= 0 and x < eval->size and y >= 0 and y < eval->size and
                        eval->board[x + 1][y + 1] == BLANK) {
                        
                        // 计算到对手棋子的距离
                        int distance = abs(i) + abs(j);
                        
                        // 计算位置分数，距离越近分数越高
                        int position_score = (3 - distance) * 1000;  // 距离1分数2000，距离2分数1000
                        int score = eval->black_scores[x][y] + eval->white_scores[x][y] + position_score;
                        
                        // 检查是否已经添加过这个位置
                        int already_added = 0;
                        for (int m = 0; m < count; m++) {
                            if (temp_moves[m].x == x && temp_moves[m].y == y) {
                                temp_moves[m].score = max(temp_moves[m].score, score);
                                already_added = 1;
                                break;
                            }
                        }
                        
                        if (!already_added) {
                            temp_moves[count].x = x;
                            temp_moves[count].y = y;
                            temp_moves[count].score = score;
                            count++;
                        }
                    }
                }
            }
        }
    }

    // 如果没有找到合适的位置，考虑棋盘中心
    if (count == 0) {
        int center = eval->size / 2;
        if (eval->board[center + 1][center + 1] == BLANK) {
            temp_moves[count].x = center;
            temp_moves[count].y = center;
            temp_moves[count].score = 1000;
            count++;
        }
    }

    // 按分数排序
    qsort(temp_moves, count, sizeof(Move), compare_moves);

    // 返回最高分的几个位置
    int max_moves = (count > MAX_MOVES) ? MAX_MOVES : count;
    *move_count = max_moves;
    for (int i = 0; i < max_moves; i++) {
        moves[i][0] = temp_moves[i].x;
        moves[i][1] = temp_moves[i].y;
    }

    // 如果是黑棋，需要检查禁手
    if (role == BLACK) {
        int temp_board[MAX_ROW][MAX_COL] = {0};
        for (int i = 0; i < MAX_ROW; i++) {
            for (int j = 0; j < MAX_COL; j++) {
                temp_board[i][j] = eval->board[i + 1][j + 1];
            }
        }

        // 过滤掉禁手位置
        int valid_count = 0;
        for (int i = 0; i < *move_count; i++) {
            if (!is_forbidden(moves[i][0], moves[i][1], temp_board)) {
                moves[valid_count][0] = moves[i][0];
                moves[valid_count][1] = moves[i][1];
                valid_count++;
            }
        }
        *move_count = valid_count;
    }
}


// 在落子后更新缓存
void update_cache(Evaluate *eval, int x, int y) {
    int min_x = (x - 4 >= 0) ? x - 4 : 0;
    int max_x = (x + 4 < eval->size) ? x + 4 : eval->size - 1;
    int min_y = (y - 4 >= 0) ? y - 4 : 0;
    int max_y = (y + 4 < eval->size) ? y + 4 : eval->size - 1;

    for (int i = min_x; i <= max_x; i++)
        for (int j = min_y; j <= max_y; j++)
            for (int d = 0; d < 4; d++)
                for (int r = 0; r < 2; r++)
                    eval->shape_cache[i][j][d][r] = -1;

}


void make_move(Evaluate *eval, int x, int y, int role) {
    eval->board[x + 1][y + 1] = role;
    update_cache(eval, x, y);
    // 待补充其他操作
}

int compare_moves(const void *a, const void *b) {
    const Move *moveA = (const Move *)a;
    const Move *moveB = (const Move *)b;
    return moveB->score - moveA->score;  // 降序排列
}

void evaluate_point(Evaluate *eval, int x, int y, int role) {
    int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
    int line[9];
    int score = 0;
    
    for (int i = 0; i < 4; i++) {
        get_line(eval, x, y, directions[i][0], directions[i][1], line);
        int shape = get_shape(line, role);
        eval->shape_cache[x][y][i][role > 0 ? 1 : 0] = shape;
        
        switch (shape) {
            case SHAPE_FIVE: score += FIVE; break;
            case SHAPE_FOUR: score += FOUR; break;
            case SHAPE_BLOCK_FOUR: score += BLOCK_FOUR; break;
            case SHAPE_THREE: score += THREE; break;
            case SHAPE_BLOCK_THREE: score += BLOCK_THREE; break;
            case SHAPE_TWO: score += TWO; break;
        }
    }
    
    if (role == BLACK) {
        eval->black_scores[x][y] = score;
    } else {
        eval->white_scores[x][y] = score;
    }
}





