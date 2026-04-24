#include "raylib.h"
#include <string.h>


#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 640
#define BOARD_SIZE 8
#define CELL_SIZE (SCREEN_HEIGHT / BOARD_SIZE)

//color of board
#define LIGHT_COLOR CLITERAL(RAYWHITE)
#define DARK_COLOR CLITERAL(DARKGRAY)

//chess pieces
typedef enum {
    EMPTY = 0,
    WHITE_PAWN = 1, WHITE_KNIGHT = 2, WHITE_BISHOP = 3, WHITE_ROOK = 4, WHITE_QUEEN = 5, WHITE_KING = 6,
    BLACK_PAWN = -1, BLACK_KNIGHT = -2, BLACK_BISHOP = -3, BLACK_ROOK = -4, BLACK_QUEEN = -5, BLACK_KING = -6,
} Piece;

// starter board
int Board[BOARD_SIZE][BOARD_SIZE] = {
    { BLACK_ROOK, BLACK_KNIGHT, BLACK_BISHOP, BLACK_QUEEN, BLACK_KING, BLACK_BISHOP, BLACK_KNIGHT, BLACK_ROOK },
    { BLACK_PAWN, BLACK_PAWN,   BLACK_PAWN,   BLACK_PAWN,  BLACK_PAWN, BLACK_PAWN,   BLACK_PAWN,   BLACK_PAWN },
    {      0,      0,        0,       0,       0,       0,        0,       0    },
    {      0,      0,        0,       0,       0,       0,        0,       0    },
    {      0,      0,        0,       0,       0,       0,        0,       0    },
    {      0,      0,        0,       0,       0,       0,        0,       0    },
    { WHITE_PAWN, WHITE_PAWN,   WHITE_PAWN,   WHITE_PAWN,  WHITE_PAWN, WHITE_PAWN,   WHITE_PAWN,   WHITE_PAWN },
    { WHITE_ROOK, WHITE_KNIGHT, WHITE_BISHOP, WHITE_QUEEN, WHITE_KING, WHITE_BISHOP, WHITE_KNIGHT, WHITE_ROOK },
};

void DrawBoard(){
    for (int row = 0; row < BOARD_SIZE; row++){
        for (int col = 0; col < BOARD_SIZE; col++){
            Color cellColor;
            if((row + col) % 2 == 0){
                cellColor = LIGHT_COLOR;
            } else {
                cellColor = DARK_COLOR;
            }
            DrawRectangle(
                col * CELL_SIZE,
                row * CELL_SIZE,
                CELL_SIZE,
                CELL_SIZE,
                cellColor
            );
        }
    }
}

void DrawLabels(){
    for (int i = 0; i < CELL_SIZE; i++){
        //alphabet collumn
        char collabel[2] = {'a' + i, '\0'};
        DrawText(collabel,
                i * CELL_SIZE + 70,
                SCREEN_HEIGHT - 18,
                14,
                (i % 2 == 0) ? LIGHT_COLOR : DARK_COLOR);
        
        //number row
        char rowLabel[2] = { '8' - i, '\0' };
        DrawText(rowLabel,
                 4,
                 i * CELL_SIZE + 4,
                 14,
                 (i % 2 == 0) ? DARK_COLOR : LIGHT_COLOR);

    }
}

int main(void) {
    // 1. Khởi tạo cửa sổ
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Chess - Part 1");
    SetTargetFPS(60);

    // 2. Vòng lặp game chính
    while (!WindowShouldClose()) {
        // --- UPDATE (phần này sẽ thêm dần) ---

        // --- DRAW ---
        BeginDrawing();
            ClearBackground(BLACK);
            DrawBoard();
            DrawLabels();
        EndDrawing();
    }

    // 3. Dọn dẹp
    CloseWindow();
    return 0;
}