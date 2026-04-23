# 🎮 Tutorial: Tạo Game Chess bằng C + Raylib

> **Mục tiêu:** Xây dựng game cờ vua hoàn chỉnh từ đầu — board, quân cờ, logic di chuyển, check/checkmate.

---

## 📦 Yêu cầu cài đặt

```bash
# Ubuntu/Debian
sudo apt install libraylib-dev

# macOS
brew install raylib

# Windows
# Tải raylib installer từ https://www.raylib.com/
```

**Compile:**
```bash
gcc chess.c -o chess -lraylib -lm
```

---

## 🗂️ Cấu trúc project

```
chess/
├── chess.c          ← File duy nhất, toàn bộ game
└── README.md
```

> Tutorial này gom tất cả vào **1 file `chess.c`** để dễ học. Mỗi Part là 1 "layer" chức năng mới.

---

# PART 1 — Setup, Window & Board Rendering

## 🧠 Khái niệm

Raylib dùng vòng lặp game cơ bản:
```
Init → [Update input → Update logic → Draw] → Cleanup
```

Board cờ vua: lưới **8×8**, ô (0,0) là góc trên trái.

---

## 💻 Code — Part 1

```c
// chess.c — PART 1: Setup & Board

#include "raylib.h"

// === CONSTANTS ===
#define SCREEN_W     640
#define SCREEN_H     640
#define BOARD_SIZE   8
#define CELL_SIZE    (SCREEN_W / BOARD_SIZE)   // 80px mỗi ô

// Màu ô sáng/tối
#define COLOR_LIGHT  CLITERAL(Color){ 240, 217, 181, 255 }   // Beige
#define COLOR_DARK   CLITERAL(Color){ 181, 136,  99, 255 }   // Brown

// === HÀM VẼ BOARD ===
void DrawBoard(void) {
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            // Ô sáng khi (row+col) chẵn, tối khi lẻ
            Color cellColor = ((row + col) % 2 == 0) ? COLOR_LIGHT : COLOR_DARK;

            DrawRectangle(
                col * CELL_SIZE,   // x pixel
                row * CELL_SIZE,   // y pixel
                CELL_SIZE,         // width
                CELL_SIZE,         // height
                cellColor
            );
        }
    }
}

// === HÀM VẼ NHÃN (a-h, 1-8) ===
void DrawLabels(void) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        // Chữ cột: a b c d e f g h
        char colLabel[2] = { 'a' + i, '\0' };
        DrawText(colLabel,
                 i * CELL_SIZE + 4,
                 SCREEN_H - 18,
                 14,
                 (i % 2 == 0) ? COLOR_DARK : COLOR_LIGHT);

        // Số hàng: 8 7 6 5 4 3 2 1
        char rowLabel[2] = { '8' - i, '\0' };
        DrawText(rowLabel,
                 4,
                 i * CELL_SIZE + 4,
                 14,
                 (i % 2 == 0) ? COLOR_DARK : COLOR_LIGHT);
    }
}

int main(void) {
    // 1. Khởi tạo cửa sổ
    InitWindow(SCREEN_W, SCREEN_H, "Chess - Part 1");
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
```

### ✅ Giải thích chi tiết

| Dòng | Giải thích |
|------|-----------|
| `#define CELL_SIZE (SCREEN_W / BOARD_SIZE)` | 640/8 = **80px** mỗi ô, tự động scale theo màn hình |
| `(row + col) % 2 == 0` | Trick cổ điển: tổng chỉ số chẵn → ô sáng, lẻ → ô tối |
| `col * CELL_SIZE, row * CELL_SIZE` | Chuyển từ **tọa độ lưới** (0–7) sang **tọa độ pixel** |
| `BeginDrawing() / EndDrawing()` | Raylib dùng **double buffering** — vẽ lên buffer ẩn, rồi flip |
| `SetTargetFPS(60)` | Giới hạn 60 FPS, tránh đốt CPU |

---

# PART 2 — Dữ liệu quân cờ & Vẽ bằng ký tự Unicode

## 🧠 Khái niệm

Lưu board dưới dạng mảng `int board[8][8]`:
- **Số dương** = quân **Trắng** (White)
- **Số âm** = quân **Đen** (Black)  
- **0** = ô trống

```
EMPTY=0, PAWN=1, KNIGHT=2, BISHOP=3, ROOK=4, QUEEN=5, KING=6
```

Dùng **Unicode Chess Symbols** (♔♕♖♗♘♙) để vẽ — không cần file ảnh!

---

## 💻 Code — Part 2

```c
// chess.c — PART 2: Piece data & Rendering

#include "raylib.h"
#include <string.h>

#define SCREEN_W   640
#define SCREEN_H   640
#define BOARD_SIZE 8
#define CELL_SIZE  (SCREEN_W / BOARD_SIZE)

#define COLOR_LIGHT  CLITERAL(Color){ 240, 217, 181, 255 }
#define COLOR_DARK   CLITERAL(Color){ 181, 136,  99, 255 }

// === PIECE TYPES ===
// Quy ước: dương = trắng, âm = đen, 0 = trống
typedef enum {
    EMPTY  =  0,
    W_PAWN =  1, W_KNIGHT =  2, W_BISHOP =  3,
    W_ROOK =  4, W_QUEEN  =  5, W_KING   =  6,
    B_PAWN = -1, B_KNIGHT = -2, B_BISHOP = -3,
    B_ROOK = -4, B_QUEEN  = -5, B_KING   = -6
} Piece;

// === BOARD STATE ===
// board[row][col], row 0 = trên cùng (phía Đen), row 7 = dưới (phía Trắng)
int board[BOARD_SIZE][BOARD_SIZE] = {
    { B_ROOK, B_KNIGHT, B_BISHOP, B_QUEEN, B_KING, B_BISHOP, B_KNIGHT, B_ROOK },
    { B_PAWN, B_PAWN,   B_PAWN,   B_PAWN,  B_PAWN, B_PAWN,   B_PAWN,   B_PAWN },
    {      0,      0,        0,       0,       0,       0,        0,       0    },
    {      0,      0,        0,       0,       0,       0,        0,       0    },
    {      0,      0,        0,       0,       0,       0,        0,       0    },
    {      0,      0,        0,       0,       0,       0,        0,       0    },
    { W_PAWN, W_PAWN,   W_PAWN,   W_PAWN,  W_PAWN, W_PAWN,   W_PAWN,   W_PAWN },
    { W_ROOK, W_KNIGHT, W_BISHOP, W_QUEEN, W_KING, W_BISHOP, W_KNIGHT, W_ROOK },
};

// === UNICODE KÝ HIỆU CỜ VUA ===
// Trắng: ♔♕♖♗♘♙  |  Đen: ♚♛♜♝♞♟
// Lưu dạng UTF-8 string
const char* PIECE_SYMBOLS[13] = {
    // Index 0–6: Black King → Black Pawn (tức |piece| 6→1 với piece âm)
    // Index 7: Empty (không dùng)
    // Index 8–12: White Pawn → White King (piece 1→5)
    // Mảng được index bằng (piece + 6) để tránh index âm
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
// Truy cập: PIECE_SYMBOLS[piece + 6]

// === VẼ BOARD ===
void DrawBoard(void) {
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            Color cellColor = ((row + col) % 2 == 0) ? COLOR_LIGHT : COLOR_DARK;
            DrawRectangle(col * CELL_SIZE, row * CELL_SIZE, CELL_SIZE, CELL_SIZE, cellColor);
        }
    }
}

// === VẼ QUÂN CỜ ===
void DrawPieces(Font font) {
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            int piece = board[row][col];
            if (piece == EMPTY) continue;

            const char* symbol = PIECE_SYMBOLS[piece + 6];

            // Màu quân: trắng hay đen?
            // Thêm viền đen để dễ nhìn trên mọi nền
            Color pieceColor = (piece > 0) ? WHITE : BLACK;
            Color shadowColor = (piece > 0) ? BLACK : DARKGRAY;

            // Tính vị trí giữa ô
            float fontSize = CELL_SIZE * 0.75f;
            Vector2 textSize = MeasureTextEx(font, symbol, fontSize, 0);
            float x = col * CELL_SIZE + (CELL_SIZE - textSize.x) / 2.0f;
            float y = row * CELL_SIZE + (CELL_SIZE - textSize.y) / 2.0f;

            // Vẽ bóng (shadow) để tạo depth
            DrawTextEx(font, symbol, (Vector2){ x + 2, y + 2 }, fontSize, 0, shadowColor);
            // Vẽ quân cờ
            DrawTextEx(font, symbol, (Vector2){ x, y }, fontSize, 0, pieceColor);
        }
    }
}

int main(void) {
    InitWindow(SCREEN_W, SCREEN_H, "Chess - Part 2");
    SetTargetFPS(60);

    // Load font hỗ trợ Unicode (font mặc định của raylib không có chess symbols)
    // Dùng font system hoặc load TTF có Unicode
    Font font = LoadFontEx("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
                            80, NULL, 0);
    // Nếu không load được, raylib dùng font default (có thể không có symbol)
    // Fallback: dùng font mặc định
    if (font.texture.id == 0) {
        font = GetFontDefault();
    }

    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(BLACK);
            DrawBoard();
            DrawPieces(font);
        EndDrawing();
    }

    UnloadFont(font);
    CloseWindow();
    return 0;
}
```

### ✅ Giải thích chi tiết

**Tại sao dùng `piece + 6` làm index?**
```
piece âm nhất = -6 (Black King)
-6 + 6 = 0  → index 0 của mảng ✓
piece dương nhất = +6 (White King)
+6 + 6 = 12 → index 12 ✓
```
Đây là trick phổ biến để "shift" range âm thành dương.

**`MeasureTextEx`** — đo kích thước text để căn giữa trong ô.

**Shadow rendering** — vẽ cùng text lệch (x+2, y+2) với màu tối trước, tạo hiệu ứng chiều sâu.

---

# PART 3 — Mouse Input & Chọn quân cờ (Selection)

## 🧠 Khái niệm

Cần tracking 2 trạng thái:
1. **Chưa chọn** → click chọn quân
2. **Đã chọn** → click ô đích để di chuyển

```
Mouse pixel (x, y) → Board cell (col, row) = (x / CELL_SIZE, y / CELL_SIZE)
```

---

## 💻 Code — Part 3

```c
// Thêm vào phần biến toàn cục (sau board[]):

// === GAME STATE ===
typedef struct {
    int selectedRow;    // Ô đang chọn (-1 nếu chưa chọn)
    int selectedCol;
    int currentTurn;    // 1 = White, -1 = Black
    bool gameOver;
} GameState;

GameState game = {
    .selectedRow = -1,
    .selectedCol = -1,
    .currentTurn = 1,    // Trắng đi trước
    .gameOver = false
};

// === VẼ Ô HIGHLIGHT ===
void DrawHighlights(void) {
    // Vẽ ô được chọn
    if (game.selectedRow != -1) {
        DrawRectangle(
            game.selectedCol * CELL_SIZE,
            game.selectedRow * CELL_SIZE,
            CELL_SIZE, CELL_SIZE,
            CLITERAL(Color){ 100, 200, 100, 120 }  // Xanh lá, trong suốt
        );
    }
}

// === XỬ LÝ CLICK CHUỘT ===
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

    int clickedPiece = board[clickRow][clickCol];

    if (game.selectedRow == -1) {
        // --- CHƯA CHỌN: thử chọn quân ---
        // Chỉ chọn được quân của lượt hiện tại
        if ((game.currentTurn == 1  && clickedPiece > 0) ||
            (game.currentTurn == -1 && clickedPiece < 0)) {
            game.selectedRow = clickRow;
            game.selectedCol = clickCol;
        }
    } else {
        // --- ĐÃ CHỌN: thử di chuyển ---
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
            // Click ô trống hoặc quân địch → thực hiện di chuyển (Part 4 sẽ validate)
            board[clickRow][clickCol] = board[game.selectedRow][game.selectedCol];
            board[game.selectedRow][game.selectedCol] = EMPTY;

            // Đổi lượt
            game.currentTurn = -game.currentTurn;
            game.selectedRow = -1;
            game.selectedCol = -1;
        }
    }
}
```

---

# PART 4 — Move Validation (Logic di chuyển hợp lệ)

## 🧠 Khái niệm

Đây là phần **phức tạp nhất**. Mỗi loại quân có luật đi riêng:

| Quân | Luật di chuyển |
|------|---------------|
| ♙ Pawn | Tiến 1 ô (hoặc 2 ô đầu), ăn chéo |
| ♘ Knight | Hình chữ L (2+1) |
| ♗ Bishop | Đường chéo, không giới hạn |
| ♖ Rook | Ngang/dọc, không giới hạn |
| ♕ Queen | Bishop + Rook |
| ♔ King | 1 ô mọi hướng |

---

## 💻 Code — Part 4

```c
// === HELPER: Kiểm tra ô trong board ===
bool InBounds(int row, int col) {
    return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
}

// === HELPER: Kiểm tra ô trống ===
bool IsEmpty(int row, int col) {
    return board[row][col] == EMPTY;
}

// === HELPER: Màu quân ===
int PieceColor(int piece) {
    if (piece > 0) return 1;   // White
    if (piece < 0) return -1;  // Black
    return 0;                   // Empty
}

// === VALIDATE MOVE: Pawn ===
bool IsValidPawnMove(int fromRow, int fromCol, int toRow, int toCol) {
    int piece = board[fromRow][fromCol];
    int dir = (piece > 0) ? -1 : 1;   // Trắng đi lên (row giảm), Đen đi xuống
    int startRow = (piece > 0) ? 6 : 1; // Hàng xuất phát để đi 2 ô

    int dr = toRow - fromRow;
    int dc = toCol - fromCol;

    // Tiến thẳng 1 ô
    if (dc == 0 && dr == dir && IsEmpty(toRow, toCol)) return true;

    // Tiến thẳng 2 ô từ vị trí ban đầu
    if (dc == 0 && dr == 2 * dir && fromRow == startRow &&
        IsEmpty(fromRow + dir, fromCol) && IsEmpty(toRow, toCol)) return true;

    // Ăn chéo
    if (abs(dc) == 1 && dr == dir &&
        !IsEmpty(toRow, toCol) &&
        PieceColor(board[toRow][toCol]) != PieceColor(piece)) return true;

    return false;
}

// === VALIDATE MOVE: Knight ===
bool IsValidKnightMove(int fromRow, int fromCol, int toRow, int toCol) {
    int dr = abs(toRow - fromRow);
    int dc = abs(toCol - fromCol);
    // Chữ L: (2,1) hoặc (1,2)
    return (dr == 2 && dc == 1) || (dr == 1 && dc == 2);
}

// === VALIDATE MOVE: Sliding pieces (Rook, Bishop, Queen) ===
// Kiểm tra xem đường đi có bị chặn không
bool IsClearPath(int fromRow, int fromCol, int toRow, int toCol) {
    int dr = (toRow > fromRow) ? 1 : (toRow < fromRow) ? -1 : 0;
    int dc = (toCol > fromCol) ? 1 : (toCol < fromCol) ? -1 : 0;

    int r = fromRow + dr;
    int c = fromCol + dc;

    // Bước dọc đường đi, kiểm tra mỗi ô trung gian
    while (r != toRow || c != toCol) {
        if (!IsEmpty(r, c)) return false;  // Bị chặn!
        r += dr;
        c += dc;
    }
    return true;
}

bool IsValidRookMove(int fromRow, int fromCol, int toRow, int toCol) {
    // Chỉ đi ngang hoặc dọc
    if (fromRow != toRow && fromCol != toCol) return false;
    return IsClearPath(fromRow, fromCol, toRow, toCol);
}

bool IsValidBishopMove(int fromRow, int fromCol, int toRow, int toCol) {
    // Chỉ đi chéo: |dr| == |dc|
    if (abs(toRow - fromRow) != abs(toCol - fromCol)) return false;
    return IsClearPath(fromRow, fromCol, toRow, toCol);
}

bool IsValidQueenMove(int fromRow, int fromCol, int toRow, int toCol) {
    // Queen = Rook + Bishop
    return IsValidRookMove(fromRow, fromCol, toRow, toCol) ||
           IsValidBishopMove(fromRow, fromCol, toRow, toCol);
}

bool IsValidKingMove(int fromRow, int fromCol, int toRow, int toCol) {
    int dr = abs(toRow - fromRow);
    int dc = abs(toCol - fromCol);
    // Chỉ 1 ô bất kỳ hướng
    return dr <= 1 && dc <= 1 && (dr + dc > 0);
}

// === MASTER VALIDATE: Tổng hợp tất cả ===
bool IsValidMove(int fromRow, int fromCol, int toRow, int toCol) {
    if (!InBounds(toRow, toCol)) return false;

    int piece = board[fromRow][fromCol];
    int targetPiece = board[toRow][toCol];

    // Không thể đi vào ô có quân cùng màu
    if (PieceColor(targetPiece) == PieceColor(piece)) return false;

    int absPiece = abs(piece);

    switch (absPiece) {
        case 1: return IsValidPawnMove(fromRow, fromCol, toRow, toCol);
        case 2: return IsValidKnightMove(fromRow, fromCol, toRow, toCol);
        case 3: return IsValidBishopMove(fromRow, fromCol, toRow, toCol);
        case 4: return IsValidRookMove(fromRow, fromCol, toRow, toCol);
        case 5: return IsValidQueenMove(fromRow, fromCol, toRow, toCol);
        case 6: return IsValidKingMove(fromRow, fromCol, toRow, toCol);
        default: return false;
    }
}
```

---

# PART 5 — Hiển thị nước đi hợp lệ (Legal Moves Highlight)

## 🧠 Khái niệm

Khi chọn một quân, **highlight tất cả ô có thể đi** — cực kỳ quan trọng cho UX.

---

## 💻 Code — Part 5

```c
// Thêm vào DrawHighlights():

void DrawHighlights(void) {
    // 1. Highlight ô được chọn
    if (game.selectedRow != -1) {
        DrawRectangle(
            game.selectedCol * CELL_SIZE,
            game.selectedRow * CELL_SIZE,
            CELL_SIZE, CELL_SIZE,
            CLITERAL(Color){ 100, 200, 100, 150 }
        );

        // 2. Highlight tất cả nước đi hợp lệ
        for (int r = 0; r < BOARD_SIZE; r++) {
            for (int c = 0; c < BOARD_SIZE; c++) {
                if (IsValidMove(game.selectedRow, game.selectedCol, r, c)) {
                    // Chấm tròn ở giữa ô
                    int cx = c * CELL_SIZE + CELL_SIZE / 2;
                    int cy = r * CELL_SIZE + CELL_SIZE / 2;

                    if (board[r][c] == EMPTY) {
                        // Ô trống: chấm nhỏ
                        DrawCircle(cx, cy, CELL_SIZE * 0.2f,
                                   CLITERAL(Color){ 50, 50, 50, 100 });
                    } else {
                        // Quân địch: viền tròn (capture hint)
                        DrawCircleLines(cx, cy, CELL_SIZE * 0.45f,
                                        CLITERAL(Color){ 200, 50, 50, 180 });
                    }
                }
            }
        }
    }
}
```

---

# PART 6 — Check Detection & King Safety

## 🧠 Khái niệm

**Check** = Vua đang bị tấn công.
**Checkmate** = Vua bị chiếu và không có nước đi nào thoát được.

Logic:
```
IsInCheck(color):
    Tìm vị trí King của color
    Duyệt tất cả quân địch
    Nếu bất kỳ quân địch nào có thể đi đến vị trí King → Check!

IsInCheckAfterMove(from, to):
    Thực hiện move tạm thời
    Kiểm tra IsInCheck
    Undo move
    Return kết quả
```

---

## 💻 Code — Part 6

```c
// === TÌM VỊ TRÍ KING ===
// Trả về vị trí King qua con trỏ
bool FindKing(int color, int *kingRow, int *kingCol) {
    int kingPiece = (color == 1) ? W_KING : B_KING;
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            if (board[r][c] == kingPiece) {
                *kingRow = r;
                *kingCol = c;
                return true;
            }
        }
    }
    return false;  // Không tìm thấy (lỗi)
}

// === KIỂM TRA CHECK ===
bool IsInCheck(int color) {
    int kingRow, kingCol;
    if (!FindKing(color, &kingRow, &kingCol)) return false;

    // Duyệt tất cả quân đối thủ
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            if (PieceColor(board[r][c]) == -color) {
                // Quân địch này có thể ăn King không?
                if (IsValidMove(r, c, kingRow, kingCol)) return true;
            }
        }
    }
    return false;
}

// === KIỂM TRA CHECK SAU KHI DI CHUYỂN ===
bool IsInCheckAfterMove(int fromRow, int fromCol, int toRow, int toCol) {
    // Backup
    int savedFrom = board[fromRow][fromCol];
    int savedTo   = board[toRow][toCol];

    // Thực hiện tạm
    board[toRow][toCol]     = savedFrom;
    board[fromRow][fromCol] = EMPTY;

    // Kiểm tra
    bool inCheck = IsInCheck(PieceColor(savedFrom));

    // Restore
    board[fromRow][fromCol] = savedFrom;
    board[toRow][toCol]     = savedTo;

    return inCheck;
}

// === VALIDATE MOVE AN TOÀN (không tự chiếu) ===
// Thay thế IsValidMove cũ
bool IsLegalMove(int fromRow, int fromCol, int toRow, int toCol) {
    if (!IsValidMove(fromRow, fromCol, toRow, toCol)) return false;
    // Không được tự đưa mình vào thế chiếu
    return !IsInCheckAfterMove(fromRow, fromCol, toRow, toCol);
}

// === KIỂM TRA CHECKMATE / STALEMATE ===
bool HasAnyLegalMove(int color) {
    for (int fr = 0; fr < BOARD_SIZE; fr++) {
        for (int fc = 0; fc < BOARD_SIZE; fc++) {
            if (PieceColor(board[fr][fc]) != color) continue;
            for (int tr = 0; tr < BOARD_SIZE; tr++) {
                for (int tc = 0; tc < BOARD_SIZE; tc++) {
                    if (IsLegalMove(fr, fc, tr, tc)) return true;
                }
            }
        }
    }
    return false;
}

// Checkmate: đang bị chiếu VÀ không có nước đi
bool IsCheckmate(int color) {
    return IsInCheck(color) && !HasAnyLegalMove(color);
}

// Stalemate: KHÔNG bị chiếu NHƯNG không có nước đi
bool IsStalemate(int color) {
    return !IsInCheck(color) && !HasAnyLegalMove(color);
}
```

---

# PART 7 — Pawn Promotion & UI hoàn chỉnh

## 🧠 Khái niệm

**Promotion**: Tốt đến hàng cuối được đổi thành Queen/Rook/Bishop/Knight.
**UI**: Hiển thị lượt đi, thông báo Check/Checkmate.

---

## 💻 Code — Part 7

```c
// === PAWN PROMOTION ===
void HandlePromotion(void) {
    // Kiểm tra tốt trắng đến hàng 0, tốt đen đến hàng 7
    for (int col = 0; col < BOARD_SIZE; col++) {
        if (board[0][col] == W_PAWN) {
            board[0][col] = W_QUEEN;  // Auto-promote thành Queen
        }
        if (board[7][col] == B_PAWN) {
            board[7][col] = B_QUEEN;
        }
    }
}

// === VẼ UI STATUS BAR ===
void DrawStatusBar(Font font) {
    // Background status bar (vẽ trên board, hoặc có thể mở rộng window)
    // Ở đây ta vẽ text overlay nhỏ ở góc

    const char* turnText = (game.currentTurn == 1) ? "WHITE's turn" : "BLACK's turn";
    Color turnColor = (game.currentTurn == 1) ? WHITE : LIGHTGRAY;

    // Vẽ semi-transparent background cho text
    DrawRectangle(0, SCREEN_H - 30, SCREEN_W, 30,
                  CLITERAL(Color){ 20, 20, 20, 200 });
    DrawText(turnText, 10, SCREEN_H - 24, 20, turnColor);

    // Trạng thái Check
    if (IsInCheck(game.currentTurn)) {
        DrawText("CHECK!", SCREEN_W - 100, SCREEN_H - 24, 20, RED);
    }

    // Game Over
    if (game.gameOver) {
        // Overlay mờ
        DrawRectangle(0, 0, SCREEN_W, SCREEN_H,
                      CLITERAL(Color){ 0, 0, 0, 160 });
        const char* msg = game.winMessage;
        int msgW = MeasureText(msg, 40);
        DrawText(msg, (SCREEN_W - msgW) / 2, SCREEN_H / 2 - 20, 40, GOLD);
        DrawText("Press R to restart", SCREEN_W/2 - 110, SCREEN_H/2 + 40, 24, WHITE);
    }
}

// Thêm vào GameState:
// char winMessage[64];

// Trong game loop, sau mỗi nước đi:
void CheckGameOver(void) {
    if (IsCheckmate(-game.currentTurn)) {
        // Người vừa đi thắng (currentTurn đã được swap)
        snprintf(game.winMessage, 64, "%s WINS!",
                 (game.currentTurn == -1) ? "WHITE" : "BLACK");
        game.gameOver = true;
    } else if (IsStalemate(game.currentTurn)) {
        snprintf(game.winMessage, 64, "STALEMATE - DRAW!");
        game.gameOver = true;
    }
}
```

---

# PART 8 — FILE HOÀN CHỈNH: `chess.c`

> Tất cả các part ghép lại thành file duy nhất, chạy được.

```c
// chess.c — Complete Chess Game in C + Raylib
// Compile: gcc chess.c -o chess -lraylib -lm

#include "raylib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

// ─────────────────────────────────────────────
// CONSTANTS
// ─────────────────────────────────────────────
#define SCREEN_W    640
#define SCREEN_H    680   // Thêm 40px cho status bar
#define BOARD_SIZE  8
#define CELL_SIZE   (SCREEN_W / BOARD_SIZE)

#define COLOR_LIGHT    CLITERAL(Color){ 240, 217, 181, 255 }
#define COLOR_DARK     CLITERAL(Color){ 181, 136,  99, 255 }
#define COLOR_SELECT   CLITERAL(Color){ 100, 200, 100, 150 }
#define COLOR_MOVE_DOT CLITERAL(Color){  50,  50,  50, 120 }
#define COLOR_CAPTURE  CLITERAL(Color){ 200,  50,  50, 180 }
#define COLOR_CHECK    CLITERAL(Color){ 220,  50,  50, 150 }

// ─────────────────────────────────────────────
// PIECE DEFINITIONS
// ─────────────────────────────────────────────
#define EMPTY    0
#define W_PAWN   1
#define W_KNIGHT 2
#define W_BISHOP 3
#define W_ROOK   4
#define W_QUEEN  5
#define W_KING   6
#define B_PAWN  -1
#define B_KNIGHT -2
#define B_BISHOP -3
#define B_ROOK  -4
#define B_QUEEN -5
#define B_KING  -6

// Unicode chess symbols (index = piece + 6)
const char* PIECE_SYMBOLS[13] = {
    "♚","♛","♜","♝","♞","♟"," ","♙","♘","♗","♖","♕","♔"
};

// ─────────────────────────────────────────────
// GAME STATE
// ─────────────────────────────────────────────
int board[BOARD_SIZE][BOARD_SIZE];
int selectedRow = -1, selectedCol = -1;
int currentTurn = 1;  // 1=White, -1=Black
bool gameOver = false;
char statusMsg[64] = "";

void ResetBoard(void) {
    int initial[8][8] = {
        {B_ROOK, B_KNIGHT, B_BISHOP, B_QUEEN, B_KING, B_BISHOP, B_KNIGHT, B_ROOK},
        {B_PAWN, B_PAWN,   B_PAWN,   B_PAWN,  B_PAWN, B_PAWN,   B_PAWN,   B_PAWN},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {W_PAWN, W_PAWN, W_PAWN, W_PAWN, W_PAWN, W_PAWN, W_PAWN, W_PAWN},
        {W_ROOK, W_KNIGHT, W_BISHOP, W_QUEEN, W_KING, W_BISHOP, W_KNIGHT, W_ROOK},
    };
    memcpy(board, initial, sizeof(initial));
    selectedRow = selectedCol = -1;
    currentTurn = 1;
    gameOver = false;
    statusMsg[0] = '\0';
}

// ─────────────────────────────────────────────
// HELPER FUNCTIONS
// ─────────────────────────────────────────────
bool InBounds(int r, int c) {
    return r >= 0 && r < 8 && c >= 0 && c < 8;
}
int PieceColor(int p) {
    return (p > 0) ? 1 : (p < 0) ? -1 : 0;
}

// ─────────────────────────────────────────────
// MOVE VALIDATION
// ─────────────────────────────────────────────
bool IsClearPath(int fr, int fc, int tr, int tc) {
    int dr = (tr > fr) ? 1 : (tr < fr) ? -1 : 0;
    int dc = (tc > fc) ? 1 : (tc < fc) ? -1 : 0;
    int r = fr + dr, c = fc + dc;
    while (r != tr || c != tc) {
        if (board[r][c] != EMPTY) return false;
        r += dr; c += dc;
    }
    return true;
}

bool IsValidMove(int fr, int fc, int tr, int tc) {
    if (!InBounds(tr, tc)) return false;
    if (fr == tr && fc == tc) return false;

    int piece = board[fr][fc];
    int target = board[tr][tc];

    if (PieceColor(target) == PieceColor(piece)) return false;

    int ap = abs(piece);
    int dr = tr - fr, dc = tc - fc;
    int adr = abs(dr), adc = abs(dc);

    switch (ap) {
        case 1: {  // Pawn
            int dir = (piece > 0) ? -1 : 1;
            int startR = (piece > 0) ? 6 : 1;
            if (dc == 0 && dr == dir && board[tr][tc] == EMPTY) return true;
            if (dc == 0 && dr == 2*dir && fr == startR &&
                board[fr+dir][fc] == EMPTY && board[tr][tc] == EMPTY) return true;
            if (adc == 1 && dr == dir && target != EMPTY &&
                PieceColor(target) != PieceColor(piece)) return true;
            return false;
        }
        case 2:  // Knight
            return (adr == 2 && adc == 1) || (adr == 1 && adc == 2);
        case 3:  // Bishop
            return adr == adc && IsClearPath(fr, fc, tr, tc);
        case 4:  // Rook
            return (fr == tr || fc == tc) && IsClearPath(fr, fc, tr, tc);
        case 5:  // Queen
            return ((fr == tr || fc == tc) || adr == adc) && IsClearPath(fr, fc, tr, tc);
        case 6:  // King
            return adr <= 1 && adc <= 1;
        default: return false;
    }
}

bool FindKing(int color, int *kr, int *kc) {
    int target = (color == 1) ? W_KING : B_KING;
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            if (board[r][c] == target) { *kr = r; *kc = c; return true; }
    return false;
}

bool IsInCheck(int color) {
    int kr, kc;
    if (!FindKing(color, &kr, &kc)) return false;
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            if (PieceColor(board[r][c]) == -color)
                if (IsValidMove(r, c, kr, kc)) return true;
    return false;
}

bool IsLegalMove(int fr, int fc, int tr, int tc) {
    if (!IsValidMove(fr, fc, tr, tc)) return false;
    // Simulate move
    int savedFrom = board[fr][fc], savedTo = board[tr][tc];
    board[tr][tc] = savedFrom; board[fr][fc] = EMPTY;
    bool check = IsInCheck(PieceColor(savedFrom));
    board[fr][fc] = savedFrom; board[tr][tc] = savedTo;
    return !check;
}

bool HasAnyLegalMove(int color) {
    for (int fr = 0; fr < 8; fr++)
        for (int fc = 0; fc < 8; fc++) {
            if (PieceColor(board[fr][fc]) != color) continue;
            for (int tr = 0; tr < 8; tr++)
                for (int tc = 0; tc < 8; tc++)
                    if (IsLegalMove(fr, fc, tr, tc)) return true;
        }
    return false;
}

// ─────────────────────────────────────────────
// INPUT HANDLING
// ─────────────────────────────────────────────
void HandleInput(void) {
    // Restart
    if (IsKeyPressed(KEY_R)) { ResetBoard(); return; }
    if (gameOver) return;
    if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return;

    Vector2 mouse = GetMousePosition();
    int cc = (int)(mouse.x / CELL_SIZE);
    int cr = (int)(mouse.y / CELL_SIZE);
    if (!InBounds(cr, cc)) return;

    int clicked = board[cr][cc];

    if (selectedRow == -1) {
        // Chọn quân
        if (PieceColor(clicked) == currentTurn) {
            selectedRow = cr; selectedCol = cc;
        }
    } else {
        if (cr == selectedRow && cc == selectedCol) {
            selectedRow = selectedCol = -1;  // Bỏ chọn
        } else if (PieceColor(clicked) == currentTurn) {
            selectedRow = cr; selectedCol = cc;  // Chọn lại
        } else if (IsLegalMove(selectedRow, selectedCol, cr, cc)) {
            // Thực hiện nước đi
            board[cr][cc] = board[selectedRow][selectedCol];
            board[selectedRow][selectedCol] = EMPTY;

            // Pawn promotion
            if (board[cr][cc] == W_PAWN && cr == 0) board[cr][cc] = W_QUEEN;
            if (board[cr][cc] == B_PAWN && cr == 7) board[cr][cc] = B_QUEEN;

            // Đổi lượt
            currentTurn = -currentTurn;
            selectedRow = selectedCol = -1;

            // Kiểm tra game over
            if (IsInCheck(currentTurn) && !HasAnyLegalMove(currentTurn)) {
                snprintf(statusMsg, 64, "%s WINS by Checkmate!",
                         currentTurn == -1 ? "WHITE" : "BLACK");
                gameOver = true;
            } else if (!IsInCheck(currentTurn) && !HasAnyLegalMove(currentTurn)) {
                snprintf(statusMsg, 64, "STALEMATE - Draw!");
                gameOver = true;
            } else if (IsInCheck(currentTurn)) {
                snprintf(statusMsg, 64, "CHECK!");
            } else {
                statusMsg[0] = '\0';
            }
        }
    }
}

// ─────────────────────────────────────────────
// RENDERING
// ─────────────────────────────────────────────
void DrawBoard(void) {
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++) {
            Color col = ((r + c) % 2 == 0) ? COLOR_LIGHT : COLOR_DARK;
            DrawRectangle(c*CELL_SIZE, r*CELL_SIZE, CELL_SIZE, CELL_SIZE, col);
        }
}

void DrawHighlights(void) {
    // Vua đang bị chiếu → highlight đỏ
    if (IsInCheck(currentTurn)) {
        int kr, kc;
        if (FindKing(currentTurn, &kr, &kc)) {
            DrawRectangle(kc*CELL_SIZE, kr*CELL_SIZE, CELL_SIZE, CELL_SIZE, COLOR_CHECK);
        }
    }

    if (selectedRow == -1) return;

    // Highlight ô được chọn
    DrawRectangle(selectedCol*CELL_SIZE, selectedRow*CELL_SIZE,
                  CELL_SIZE, CELL_SIZE, COLOR_SELECT);

    // Highlight nước đi hợp lệ
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (!IsLegalMove(selectedRow, selectedCol, r, c)) continue;
            int cx = c*CELL_SIZE + CELL_SIZE/2;
            int cy = r*CELL_SIZE + CELL_SIZE/2;
            if (board[r][c] == EMPTY) {
                DrawCircle(cx, cy, CELL_SIZE*0.18f, COLOR_MOVE_DOT);
            } else {
                DrawRing((Vector2){cx, cy},
                         CELL_SIZE*0.38f, CELL_SIZE*0.46f,
                         0, 360, 20, COLOR_CAPTURE);
            }
        }
    }
}

void DrawPieces(Font font) {
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int piece = board[r][c];
            if (piece == EMPTY) continue;

            const char* sym = PIECE_SYMBOLS[piece + 6];
            float sz = CELL_SIZE * 0.72f;
            Vector2 ts = MeasureTextEx(font, sym, sz, 0);
            float x = c*CELL_SIZE + (CELL_SIZE - ts.x) * 0.5f;
            float y = r*CELL_SIZE + (CELL_SIZE - ts.y) * 0.5f;

            Color shadow = CLITERAL(Color){0,0,0,100};
            Color fg = (piece > 0)
                ? CLITERAL(Color){255,255,255,255}
                : CLITERAL(Color){20,20,20,255};

            DrawTextEx(font, sym, (Vector2){x+2, y+2}, sz, 0, shadow);
            DrawTextEx(font, sym, (Vector2){x,   y  }, sz, 0, fg);
        }
    }
}

void DrawStatusBar(void) {
    int barY = BOARD_SIZE * CELL_SIZE;
    DrawRectangle(0, barY, SCREEN_W, 40, CLITERAL(Color){30,30,30,255});

    // Lượt đi
    const char* turnStr = (currentTurn == 1) ? "⬜ White's Turn" : "⬛ Black's Turn";
    DrawText(turnStr, 12, barY + 11, 20,
             (currentTurn == 1) ? WHITE : LIGHTGRAY);

    // Trạng thái (Check / Game Over)
    if (statusMsg[0] != '\0') {
        Color msgColor = gameOver ? GOLD : RED;
        int w = MeasureText(statusMsg, 20);
        DrawText(statusMsg, SCREEN_W - w - 12, barY + 11, 20, msgColor);
    }

    if (gameOver) {
        DrawRectangle(0, 0, SCREEN_W, BOARD_SIZE*CELL_SIZE,
                      CLITERAL(Color){0,0,0,160});
        int w = MeasureText(statusMsg, 36);
        DrawText(statusMsg, (SCREEN_W-w)/2, SCREEN_H/2 - 40, 36, GOLD);
        DrawText("Press [R] to Restart", SCREEN_W/2 - 120, SCREEN_H/2 + 10, 22, WHITE);
    }
}

// ─────────────────────────────────────────────
// MAIN
// ─────────────────────────────────────────────
int main(void) {
    InitWindow(SCREEN_W, SCREEN_H, "♔ Chess in C + Raylib");
    SetTargetFPS(60);

    // Load font Unicode (thử nhiều path phổ biến)
    Font font = {0};
    const char* fontPaths[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/System/Library/Fonts/Supplemental/Arial Unicode.ttf",
        "C:/Windows/Fonts/seguisym.ttf",
        NULL
    };
    for (int i = 0; fontPaths[i] != NULL; i++) {
        font = LoadFontEx(fontPaths[i], 80, NULL, 0);
        if (font.texture.id != 0) break;
    }
    if (font.texture.id == 0) font = GetFontDefault();

    ResetBoard();

    while (!WindowShouldClose()) {
        HandleInput();

        BeginDrawing();
            ClearBackground(DARKGRAY);
            DrawBoard();
            DrawHighlights();
            DrawPieces(font);
            DrawStatusBar();
        EndDrawing();
    }

    if (font.texture.id != 0) UnloadFont(font);
    CloseWindow();
    return 0;
}
```

---

# 🎯 Tóm tắt kiến trúc

```
chess.c
├── Constants & Definitions
│   ├── Screen/Board sizes
│   └── Piece values (dương=trắng, âm=đen)
│
├── Game State
│   ├── board[8][8]        ← Trạng thái bàn cờ
│   ├── selectedRow/Col    ← Quân đang được chọn
│   ├── currentTurn        ← Lượt hiện tại
│   └── gameOver / msg     ← Trạng thái kết thúc
│
├── Move Validation
│   ├── IsValidMove()      ← Kiểm tra luật từng quân
│   ├── IsInCheck()        ← Phát hiện chiếu
│   ├── IsLegalMove()      ← Valid + không tự chiếu
│   └── HasAnyLegalMove()  ← Checkmate/Stalemate detection
│
├── Input Handling
│   └── HandleInput()      ← Click → chọn/di chuyển
│
└── Rendering
    ├── DrawBoard()         ← Vẽ ô cờ
    ├── DrawHighlights()    ← Highlight chọn + nước đi
    ├── DrawPieces()        ← Vẽ quân bằng Unicode
    └── DrawStatusBar()     ← UI lượt đi / check / win
```

---

# 🚀 Mở rộng tiếp theo

Khi đã nắm vững code trên, bạn có thể thêm:

| Feature | Độ khó | Gợi ý |
|---------|--------|-------|
| **En Passant** | ⭐⭐ | Track `lastPawnDoubleMove` |
| **Castling** | ⭐⭐ | Track `hasMoved[King/Rook]` |
| **Pawn Promotion Menu** | ⭐⭐ | UI chọn Queen/Rook/Bishop/Knight |
| **Move History** | ⭐⭐⭐ | Stack `Move history[]` |
| **Undo Move** | ⭐⭐⭐ | Pop từ history stack |
| **AI (Minimax)** | ⭐⭐⭐⭐ | Alpha-beta pruning |
| **Piece textures** | ⭐⭐ | `LoadTexture()` thay Unicode |
| **Animations** | ⭐⭐⭐ | Lerp vị trí quân cờ |

---

*Happy coding! 🎮*