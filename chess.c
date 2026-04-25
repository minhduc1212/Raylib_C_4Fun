#include "raylib.h"
#include <string.h>
#include <stdlib.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 640
#define BOARD_SIZE 8
#define CELL_SIZE (SCREEN_HEIGHT / BOARD_SIZE)

//color of board
#define LIGHT_COLOR CLITERAL(RAYWHITE)
#define DARK_COLOR CLITERAL(DARKGRAY)

//game state
typedef struct {
    int selectedRow;
    int selectedCol;
    int currentTurn;
    bool gameOver;
} GameState;

//gamestate in the begin
GameState game = {
    .selectedRow = -1,
    .selectedCol = -1,
    .currentTurn = 1,
    .gameOver = false,
};

//chess pieces
// black is negative and white is positive for the more understand
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

//highlight draw when choose
void DrawHightlights(){
    if (game.selectedRow != -1){
        DrawRectangle(
            game.selectedCol * CELL_SIZE,
            game.selectedRow * CELL_SIZE,
            CELL_SIZE, CELL_SIZE,
            CLITERAL(Color){ 100, 200, 100, 120 }  //
        );
    };
};

void HandleMouseInput(void) {
    if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return;
    if (game.gameOver) return;

    // Chuyển pixel → cell
    Vector2 mouse = GetMousePosition();
    int clickCol = (int)(mouse.x / CELL_SIZE);
    int clickRow = (int)(mouse.y / CELL_SIZE);

    // Kiểm tra trong board
    if (clickCol < 0 || clickCol >= BOARD_SIZE ||
        clickRow < 0 || clickRow >= BOARD_SIZE) return;

    int clickedPiece = Board[clickRow][clickCol];

    if (game.selectedRow == -1) {
        // --- CHƯA CHỌN: thử chọn quân ---
        // Chỉ chọn được quân của lượt hiện tại
        if ((game.currentTurn == 1  && clickedPiece > 0) ||
            (game.currentTurn == -1 && clickedPiece < 0)) {
            game.selectedRow = clickRow;
            game.selectedCol = clickCol;
        }
    } else {
        if (clickRow == game.selectedRow && clickCol == game.selectedCol) {
            // Click lại ô đang chọn → bỏ chọn
            game.selectedRow = -1;
            game.selectedCol = -1;
        } else if ((game.currentTurn == 1  && clickedPiece > 0) ||
                   (game.currentTurn == -1 && clickedPiece < 0)) {
            // Click quân cùng màu → chuyển sang chọn quân đó
            game.selectedRow = clickRow;
            game.selectedCol = clickCol;
        } else {
            // Click ô trống hoặc quân địch → thực hiện di chuyển 
            Board[clickRow][clickCol] = Board[game.selectedRow][game.selectedCol];
            Board[game.selectedRow][game.selectedCol] = EMPTY;

            // Đổi lượt
            game.currentTurn = -game.currentTurn;
            game.selectedRow = -1;
            game.selectedCol = -1;
        }
    }
}

const char* PIECE_SYMBOLS[13] = {
    "♚",  // -6: Black King    → index 0
    "♛",  // -5: Black Queen   → index 1
    "♜",  // -4: Black Rook    → index 2
    "♝",  // -3: Black Bishop  → index 3
    "♞",  // -2: Black Knight  → index 4
    "♟",  // -1: Black Pawn    → index 5
    " ",  //  0: Empty         → index 6
    "♙",  //  1: White Pawn    → index 7
    "♘",  //  2: White Knight  → index 8
    "♗",  //  3: White Bishop  → index 9
    "♖",  //  4: White Rook    → index 10
    "♕",  //  5: White Queen   → index 11
    "♔",  //  6: White King    → index 12
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
    for (int i = 0; i < BOARD_SIZE; i++){
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

void DrawPieces(Font font){
    for (int row = 0; row < BOARD_SIZE; row++){
        for (int col = 0; col < BOARD_SIZE; col++){
            int piece = Board[row][col];
            if (piece == EMPTY) continue;
            
            const char* symbol = PIECE_SYMBOLS[piece + 6];

            Color pieceColor;
            if (piece > 0){
                pieceColor = WHITE;
            } else {
                pieceColor = BLACK;
            }

            Color shadowColor;
            if (piece > 0){
                shadowColor = BLACK;
            } else {
                shadowColor = DARKGRAY;
            }

            //position of piece
            float fontSize = CELL_SIZE * 0.75f;
            Vector2 textSize = MeasureTextEx(font, symbol, fontSize, 0);
            float x = col * CELL_SIZE + (CELL_SIZE - textSize.x) / 2.0f;
            float y = row * CELL_SIZE + (CELL_SIZE - textSize.y) / 2.0f;

            //draw shadow
            DrawTextEx(font, symbol, (Vector2){ x + 2, y + 2 }, fontSize, 0, shadowColor);
            // Vẽ quân cờ
            DrawTextEx(font, symbol, (Vector2){ x, y }, fontSize, 0, pieceColor);


        };  
    };
};


int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Chess");
    SetTargetFPS(60);

    // 1. Prepare the codepoints we want the font to support
    // Standard ASCII (95 chars) + Chess Pieces (12 chars)
    int codepointCount = 95 + 12;
    int *codepoints = (int *)malloc(codepointCount * sizeof(int));

    // Load standard text characters (Space ' ' up to '~') for your row/col labels
    for (int i = 0; i < 95; i++) {
        codepoints[i] = 32 + i;
    }

    // Load the Unicode hex values for the chess pieces (U+2654 to U+265F)
    int chessStart = 0x2654;
    for (int i = 0; i < 12; i++) {
        codepoints[95 + i] = chessStart + i;
    }

    // 2. Load a LOCAL font file using the custom codepoints. 
    Font font = LoadFontEx("C:/Windows/Fonts/seguisym.ttf", 80, codepoints, codepointCount);
    
    // Clean up our temporary codepoint array
    free(codepoints);

    // Fallback warning if the file is missing
    if (font.glyphCount < 100) {
    TraceLog(LOG_WARNING, "Font missing glyphs!");
    font = GetFontDefault();
    }

    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(BLACK);
            DrawBoard();
            DrawLabels();
            DrawPieces(font);
        EndDrawing();
    }

    UnloadFont(font);
    CloseWindow();
    return 0;
}