#include "raylib.h"

int main(void)
{
    // -------------------------------------------------------------------------
    // 1. Initialization
    // -------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    // Open a window with a specific width, height, and title
    InitWindow(screenWidth, screenHeight, "My First Raylib Game");

    // Define our player (a square)
    Vector2 playerPosition = { (float)screenWidth / 2, (float)screenHeight / 2 };
    float playerSpeed = 5.0f;

    // Set our game to run at 60 frames-per-second
    SetTargetFPS(60);               

    while (!WindowShouldClose())    
    {
        // --- A. UPDATE (Game Logic & Input) ---
        
        if (IsKeyDown(KEY_RIGHT)) playerPosition.x += playerSpeed;
        if (IsKeyDown(KEY_LEFT)) playerPosition.x -= playerSpeed;
        if (IsKeyDown(KEY_UP)) playerPosition.y -= playerSpeed;
        if (IsKeyDown(KEY_DOWN)) playerPosition.y += playerSpeed;

        // --- B. DRAW (Rendering) ---
        BeginDrawing();
        
            // Always clear the background first, otherwise the screen gets messy
            ClearBackground(RAYWHITE);

            // Draw some instructional text
            DrawText("Move the square with arrow keys!", 10, 10, 20, DARKGRAY);

            // Draw our player rectangle at the updated X/Y position
            DrawRectangleV(playerPosition, (Vector2){ 50, 50 }, MAROON);

        EndDrawing();
    }

    // -------------------------------------------------------------------------
    // 3. De-Initialization
    // -------------------------------------------------------------------------
    CloseWindow(); // Close window and OpenGL context safely

    return 0;
}