//
// Created by romel on 2024/11/23.
//

#include "hash.h"
#include "common.h"

void init_hash_table(void) {
    // 简单的初始化，如果不需要复杂的哈希表可以留空
}

/* Compute hash value for current board state */
unsigned long long compute_board_hash(Evaluate *eval) {
    unsigned long long hash = 0;
    
    for (int i = 0; i < MAX_ROW; i++) {
        for (int j = 0; j < MAX_COL; j++) {
            int piece = eval->board[i + 1][j + 1];
            if (piece != BLANK) {
                hash = hash * 31 + (i * MAX_COL + j) * piece;
            }
        }
    }
    
    return hash;
}
