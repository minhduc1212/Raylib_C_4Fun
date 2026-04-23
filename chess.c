#include <stdio.h>
#include <stdlib.h>
#include <raygui.h>
#include <raylib.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 640
#define BOARD_SIZE 8
#define CELL_SIZE (SCREEN_HEIGHT / BOARD_SIZE)

//color of board
#define LIGHT_COLOR CLITERAL(RAYWHITE)
#define DARK_COLOR CLITERAL(DARKGRAY)

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