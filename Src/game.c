//
// Created by romel on 2024/9/28.
//

#include "game.h"
#include "minimax.h"
#include "hash.h"

extern unsigned long long current_board_hash;
int AI_ROLE;
int HUMAN_ROLE;
Evaluate evaluator;

/* Initialize menu */
void menu(void) {
    printf("***************************************\n");
    printf("***************************************\n");
    printf("******* Welcome to Gobang Game! *******\n");
    printf("******     Developer: ËïÞÈ·É     *******\n");
    printf("***     Please enter your choice    ***\n");
    printf("*      1. Person versus Person        *\n");
    printf("*        2. Person versus AI          *\n");
    printf("*              3. Quit                *\n");
    printf("***************************************\n");
    printf("***************************************\n");
    printf("Please enter your choice:\n");
}

/* Draw chess piece */
void draw_chessman(int type, char *tableline) {
    if (type == WHITE)
        printf("¡ñ  ");
    else if (type == BLACK)
        printf("¡ð  ");
    else if (type == RECENT_WHITE)
        printf("¡ø  ");  // Recent white piece
    else if (type == RECENT_BLACK)
        printf("¡÷  ");  // Recent black piece
    else if (type == BLANK)
        printf("%s", tableline);  // Empty position
}

/* Draw chessboard */
void draw_chessboard(int row, int col, int chessboard[][MAX_COL]) {
    int i, j;

    for (i = 0; i < row; i++) {
        // Draw row numbers
        printf("%2d ", i + 1);
        for (j = 0; j < col; j++) {
            if (j == 0) {
                if (i == 0)
                    draw_chessman(chessboard[i][j], "©°  ");
                else if (i == row - 1)
                    draw_chessman(chessboard[i][j], "©¸  ");
                else
                    draw_chessman(chessboard[i][j], "©À  ");
            }
            else if (j == col - 1) {
                if (i == 0)
                    draw_chessman(chessboard[i][j], "©´");
                else if (i == row - 1)
                    draw_chessman(chessboard[i][j], "©¼");
                else
                    draw_chessman(chessboard[i][j], "©È");
            }
            else {
                if (i == 0)
                    draw_chessman(chessboard[i][j], "©Ð  ");
                else if (i == row - 1)
                    draw_chessman(chessboard[i][j], "©Ø  ");
                else
                    draw_chessman(chessboard[i][j], "©à  ");
            }
        }
        printf("\n");
    }
    // Draw column letters
    printf("  ");
    for (j = 0; j < col; j++) {
        printf(" %c ", 'A' + j);
    }
    printf("\n");
}

/* Check win condition: returns BLACK(1) for black win, WHITE(-1) for white win */
int is_win(int chessboard[][MAX_COL]) {
    int directions[4][2] = {
        {0, 1},  // Horizontal
        {1, 0},  // Vertical
        {1, 1},  // Diagonal
        {1, -1}  // Anti-diagonal
    };
    int i, j, k;

    for (i = 0; i < MAX_ROW; i++) {
        for (j = 0; j < MAX_COL; j++) {
            int type = chessboard[i][j];
            // Skip empty positions
            if (type == BLANK)
                continue;
            // Convert RECENT pieces to their normal counterparts
            if (type == RECENT_BLACK or type == RECENT_WHITE)
                type = (type == RECENT_BLACK) ? BLACK : WHITE;

            // Check all directions
            for (k = 0; k < 4; k++) {
                int count = 1;  // Count consecutive pieces
                int dx = directions[k][0];
                int dy = directions[k][1];
                int x, y;

                // Check forward direction
                x = i + dx;
                y = j + dy;
                while (x >= 0 and x < MAX_ROW and y >= 0 and y < MAX_COL) {
                    int temp_type = chessboard[x][y];
                    if (temp_type == RECENT_BLACK or temp_type == RECENT_WHITE)
                        temp_type = (temp_type == RECENT_BLACK) ? BLACK : WHITE;
                    if (temp_type == type)
                        count++;
                    else
                        break;
                    x += dx;
                    y += dy;
                }

                // Check backward direction
                x = i - dx;
                y = j - dy;
                while (x >= 0 and x < MAX_ROW and y >= 0 and y < MAX_COL) {
                    int temp_type = chessboard[x][y];
                    if (temp_type == RECENT_BLACK or temp_type == RECENT_WHITE)
                        temp_type = (temp_type == RECENT_BLACK) ? BLACK : WHITE;
                    if (temp_type == type)
                        count++;
                    else
                        break;
                    x -= dx;
                    y -= dy;
                }

                if (count >= 5)
                    return type;
            }
        }
    }
    return BLANK;  // No winner yet
}

/* Check if board is full */
int is_full(int chessboard[][MAX_COL], int row, int col) {
    int i, j;
    for (i = 0; i < row; i++) {
        for (j = 0; j < col; j++) {
            if (chessboard[i][j] == BLANK) {
                return 0;
            }
        }
    }
    return 1;
}

/* Person vs Person game mode */
void person_vs_person(void) {
    int chessboard[MAX_ROW][MAX_COL] = {BLANK};
    int i, j;
    int step;
    char input[4];
    draw_chessboard(MAX_ROW, MAX_COL, chessboard);

    for (step = 1; step <= MAX_ROW * MAX_COL; step++) {
        // Black's turn (odd steps)
        if (step % 2 == 1) {
            printf("Please place the black chess piece (e.g., H8, A12):\n");
            while (1) {
                if (scanf("%s", input) == 1) {
                    j = input[0] - 'A';
                    i = atoi(&input[1]) - 1;
                    if (chessboard[i][j] != BLANK) {
                        printf("Position already occupied, please try again.\n");
                        continue;
                    }
                    if (i >= MAX_ROW or j >= MAX_COL or i < 0 or j < 0) {
                        printf("Position out of bounds, please try again.\n");
                        continue;
                    }
                }
                else {
                    printf("Invalid input.\n");
                    while(getchar() != '\n');
                    continue;
                }
                break;
            }
            chessboard[i][j] = RECENT_BLACK;
            draw_chessboard(MAX_ROW, MAX_COL, chessboard);

            if (is_forbidden(i, j, chessboard)) {
                printf("Black player made a forbidden move. White wins!\n");
                exit(0);
            }
            if (is_win(chessboard) == BLACK) {
                printf("Black wins!\n");
                exit(0);
            }
            if (is_full(chessboard, MAX_ROW, MAX_COL)) {
                printf("Draw game!\n");
                exit(0);
            }
            for (int x = 0; x < MAX_ROW; x++) {
                for (int y = 0; y < MAX_COL; y++) {
                    if (chessboard[x][y] == RECENT_BLACK)
                        chessboard[x][y] = BLACK;
                }
            }
        }
        // White's turn (even steps)
        else {
            printf("Please place the white chess piece:\n");
            while (1) {
                if (scanf("%s", input) == 1) {
                    j = input[0] - 'A';
                    i = atoi(&input[1]) - 1;
                    if (chessboard[i][j] != BLANK) {
                        printf("Position already occupied, please try again.\n");
                        continue;
                    }
                    if (i >= MAX_ROW or j >= MAX_COL or i < 0 or j < 0) {
                        printf("Position out of bounds, please try again.\n");
                        continue;
                    }
                }
                else {
                    printf("Invalid input.\n");
                    while(getchar() != '\n');
                    continue;
                }
                break;
            }
            chessboard[i][j] = RECENT_WHITE;
            draw_chessboard(MAX_ROW, MAX_COL, chessboard);
            if (is_win(chessboard) == WHITE) {
                printf("White wins!\n");
                exit(0);
            }
            if (is_full(chessboard, MAX_ROW, MAX_COL)) {
                printf("Draw game!\n");
                exit(0);
            }
            for (int x = 0; x < MAX_ROW; x++) {
                for (int y = 0; y < MAX_COL; y++) {
                    if (chessboard[x][y] == RECENT_WHITE)
                        chessboard[x][y] = WHITE;
                }
            }
        }
    }
}

/* Person vs AI game mode */
void person_vs_ai(void) {
    int chessboard[MAX_ROW][MAX_COL];
    memset(chessboard, BLANK, sizeof(chessboard));
    int i, j;
    int step;
    char input[4];
    int current_role;

    // Let player choose color
    select_player_color();

    // Initialize evaluator and hash table
    init_evaluate(&evaluator);
    init_hash_table();

    // Draw initial board
    draw_chessboard(MAX_ROW, MAX_COL, chessboard);

    // Determine first player
    current_role = (HUMAN_ROLE == BLACK) ? BLACK : AI_ROLE;

    for (step = 1; step <= MAX_ROW * MAX_COL; step++) {
        if (current_role == HUMAN_ROLE) {
            // Human's turn
            printf("Your turn (%s). Enter position (e.g., H8, A12):\n", 
                   HUMAN_ROLE == BLACK ? "BLACK" : "WHITE");
            while (1) {
                if (scanf("%s", input) == 1) {
                    j = input[0] - 'A';
                    i = atoi(&input[1]) - 1;
                    if (chessboard[i][j] != BLANK) {
                        printf("Position already occupied, please try again.\n");
                        continue;
                    }
                    if (i >= MAX_ROW or j >= MAX_COL or i < 0 or j < 0) {
                        printf("Position out of bounds, please try again.\n");
                        continue;
                    }
                }
                else {
                    printf("Invalid input.\n");
                    while(getchar() != '\n');
                    continue;
                }
                break;
            }

            chessboard[i][j] = (HUMAN_ROLE == BLACK) ? RECENT_BLACK : RECENT_WHITE;
            emplace_chess(&evaluator, i, j, HUMAN_ROLE);
            draw_chessboard(MAX_ROW, MAX_COL, chessboard);

            if (HUMAN_ROLE == BLACK and is_forbidden(i, j, chessboard)) {
                printf("Forbidden move! White wins!\n");
                exit(0);
            }

            if (is_win(chessboard) == HUMAN_ROLE) {
                printf("%s wins!\n", HUMAN_ROLE == BLACK ? "BLACK" : "WHITE");
                exit(0);
            }
        }
        else {
            // AI's turn
            printf("\nAI is thinking...\n");

            Path best_path = {.length = 0};
            int value = iterative_deepening(&evaluator, MIN_SCORE, MAX_SCORE, AI_ROLE, &best_path);

            if (value == MIN_SCORE) {
                printf("AI has no valid moves.\n");
                exit(0);
            }

            int ai_x = best_path.steps[0][0];
            int ai_y = best_path.steps[0][1];

            chessboard[ai_x][ai_y] = (AI_ROLE == BLACK) ? RECENT_BLACK : RECENT_WHITE;
            emplace_chess(&evaluator, ai_x, ai_y, AI_ROLE);
            draw_chessboard(MAX_ROW, MAX_COL, chessboard);
            printf("AI places at %c%d\n", 'A' + ai_y, 15 - ai_x);

            if (is_win(chessboard) == AI_ROLE) {
                printf("AI (%s) wins!\n", AI_ROLE == BLACK ? "BLACK" : "WHITE");
                exit(0);
            }
        }

        if (is_full(chessboard, MAX_ROW, MAX_COL)) {
            printf("Draw game!\n");
            exit(0);
        }

        // Update recent moves to normal pieces
        for (int x = 0; x < MAX_ROW; x++) {
            for (int y = 0; y < MAX_COL; y++) {
                if (chessboard[x][y] == RECENT_BLACK)
                    chessboard[x][y] = BLACK;
                else if (chessboard[x][y] == RECENT_WHITE)
                    chessboard[x][y] = WHITE;
            }
        }
        
        // Switch roles
        current_role = -current_role;
    }
}

/* Let player select color */
void select_player_color(void) {
    int choice;
    printf("\nSelect your color:\n");
    printf("1. Black (First Move)\n");
    printf("2. White (Second Move)\n");
    while (1) {
        if (scanf("%d", &choice) == 1) {
            if (choice == 1 or choice == 2) {
                HUMAN_ROLE = (choice == 1) ? BLACK : WHITE;
                AI_ROLE = -HUMAN_ROLE;
                break;
            }
        }
        printf("Invalid choice, please select 1 or 2.\n");
        while (getchar() != '\n');
    }
}
