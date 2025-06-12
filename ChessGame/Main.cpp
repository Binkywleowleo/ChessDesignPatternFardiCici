#include "raylib.h"
#include <vector>
#include <memory>
#include <map>
#include <string>
#include <iostream>
#include <stack>

constexpr int BOARD_SIZE = 8;
constexpr int TILE_SIZE = 80;

enum class PieceType {
    None = 0,
    Rook,
    Knight,
    Bishop,
    Queen,
    King,
    Pawn
};

enum class PieceColor {
    None = 0,
    Black,
    White
};

struct Vector2Int {
    int x;
    int y;

    Vector2Int(int _x = 0, int _y = 0) : x(_x), y(_y) {}

    bool operator==(const Vector2Int& other) const {
        return x == other.x && y == other.y;
    }
    bool operator!=(const Vector2Int& other) const {
        return !(*this == other);
    }
};

class Command {
public:
    virtual ~Command() = default;
    virtual void Execute() = 0;
    virtual void Undo() = 0;
};

class Piece {
public:
    PieceType type;
    PieceColor color;
    Vector2Int boardPosition;
    bool hasMoved;

    Piece(PieceType t, PieceColor c, Vector2Int pos)
        : type(t), color(c), boardPosition(pos), hasMoved(false) {}

    virtual ~Piece() {}

    virtual std::vector<Vector2Int> GetValidMoves(const std::vector<std::vector<std::unique_ptr<Piece>>>& board) = 0;

    static bool InBounds(int x, int y) {
        return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
    }

    static bool CanMoveTo(const std::vector<std::vector<std::unique_ptr<Piece>>>& board, int x, int y, PieceColor ownColor) {
        if (!InBounds(x, y)) return false;
        if (!board[y][x]) return true;
        return board[y][x]->color != ownColor;
    }

    virtual bool IsPromotion() const { return false; }

    virtual std::unique_ptr<Piece> Clone() const = 0;
};

class Rook : public Piece {
public:
    Rook(PieceColor c, Vector2Int pos) : Piece(PieceType::Rook, c, pos) {}

    std::vector<Vector2Int> GetValidMoves(const std::vector<std::vector<std::unique_ptr<Piece>>>& board) override {
        std::vector<Vector2Int> moves;
        int dirs[4][2] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
        for (auto& d : dirs) {
            for (int step = 1; step < BOARD_SIZE; ++step) {
                int nx = boardPosition.x + d[0] * step;
                int ny = boardPosition.y + d[1] * step;
                if (!InBounds(nx, ny)) break;
                if (!board[ny][nx]) {
                    moves.emplace_back(nx, ny);
                }
                else {
                    if (board[ny][nx]->color != color)
                        moves.emplace_back(nx, ny);
                    break;
                }
            }
        }
        return moves;
    }
    std::unique_ptr<Piece> Clone() const override {
        return std::make_unique<Rook>(*this);
    }
};

class Knight : public Piece {
public:
    Knight(PieceColor c, Vector2Int pos) : Piece(PieceType::Knight, c, pos) {}

    std::vector<Vector2Int> GetValidMoves(const std::vector<std::vector<std::unique_ptr<Piece>>>& board) override {
        std::vector<Vector2Int> moves;
        int jumps[8][2] = {
            {1,2}, {2,1}, {-1,2}, {-2,1},
            {1,-2}, {2,-1}, {-1,-2}, {-2,-1}
        };
        for (auto& j : jumps) {
            int nx = boardPosition.x + j[0];
            int ny = boardPosition.y + j[1];
            if (InBounds(nx, ny)) {
                if (!board[ny][nx] || board[ny][nx]->color != color)
                    moves.emplace_back(nx, ny);
            }
        }
        return moves;
    }

    std::unique_ptr<Piece> Clone() const override {
        return std::make_unique<Knight>(*this);
    }
};

class Bishop : public Piece {
public:
    Bishop(PieceColor c, Vector2Int pos) : Piece(PieceType::Bishop, c, pos) {}

    std::vector<Vector2Int> GetValidMoves(const std::vector<std::vector<std::unique_ptr<Piece>>>& board) override {
        std::vector<Vector2Int> moves;
        int dirs[4][2] = { {1,1}, {1,-1}, {-1,1}, {-1,-1} };
        for (auto& d : dirs) {
            for (int step = 1; step < BOARD_SIZE; ++step) {
                int nx = boardPosition.x + d[0] * step;
                int ny = boardPosition.y + d[1] * step;
                if (!InBounds(nx, ny)) break;
                if (!board[ny][nx]) {
                    moves.emplace_back(nx, ny);
                }
                else {
                    if (board[ny][nx]->color != color)
                        moves.emplace_back(nx, ny);
                    break;
                }
            }
        }
        return moves;
    }

    std::unique_ptr<Piece> Clone() const override {
        return std::make_unique<Bishop>(*this);
    }
};

class Queen : public Piece {
public:
    Queen(PieceColor c, Vector2Int pos) : Piece(PieceType::Queen, c, pos) {}

    std::vector<Vector2Int> GetValidMoves(const std::vector<std::vector<std::unique_ptr<Piece>>>& board) override {
        std::vector<Vector2Int> moves;
        int dirs[8][2] = {
            {1,0}, {-1,0}, {0,1}, {0,-1},
            {1,1}, {1,-1}, {-1,1}, {-1,-1}
        };
        for (auto& d : dirs) {
            for (int step = 1; step < BOARD_SIZE; ++step) {
                int nx = boardPosition.x + d[0] * step;
                int ny = boardPosition.y + d[1] * step;
                if (!InBounds(nx, ny)) break;
                if (!board[ny][nx]) {
                    moves.emplace_back(nx, ny);
                }
                else {
                    if (board[ny][nx]->color != color)
                        moves.emplace_back(nx, ny);
                    break;
                }
            }
        }
        return moves;
    }

    std::unique_ptr<Piece> Clone() const override {
        return std::make_unique<Queen>(*this);
    }
};

class King : public Piece {
public:
    King(PieceColor c, Vector2Int pos) : Piece(PieceType::King, c, pos) {}

    std::vector<Vector2Int> GetValidMoves(const std::vector<std::vector<std::unique_ptr<Piece>>>& board) override {
        std::vector<Vector2Int> moves;
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                if (dx == 0 && dy == 0) continue;
                int nx = boardPosition.x + dx;
                int ny = boardPosition.y + dy;
                if (InBounds(nx, ny)) {
                    if (!board[ny][nx] || board[ny][nx]->color != color) {
                        moves.emplace_back(nx, ny);
                    }
                }
            }
        }
        return moves;
    }

    std::unique_ptr<Piece> Clone() const override {
        return std::make_unique<King>(*this);
    }
};

class Pawn : public Piece {
public:
    Pawn(PieceColor c, Vector2Int pos) : Piece(PieceType::Pawn, c, pos) {}

    std::vector<Vector2Int> GetValidMoves(const std::vector<std::vector<std::unique_ptr<Piece>>>& board) override {
        std::vector<Vector2Int> moves;
        int direction = (color == PieceColor::White) ? -1 : 1;
        int startRow = (color == PieceColor::White) ? 6 : 1;
        int x = boardPosition.x;
        int y = boardPosition.y;

        
        if (InBounds(x, y + direction) && !board[y + direction][x]) {
            moves.emplace_back(x, y + direction);
            
            if (y == startRow && InBounds(x, y + 2 * direction) && !board[y + 2 * direction][x]) {
                moves.emplace_back(x, y + 2 * direction);
            }
        }

        
        for (int dx : {-1, 1}) {
            int nx = x + dx;
            int ny = y + direction;
            if (InBounds(nx, ny) && board[ny][nx] && board[ny][nx]->color != color) {
                moves.emplace_back(nx, ny);
            }
        }

        return moves;
    }

    bool IsPromotion() const override {
        return (color == PieceColor::White && boardPosition.y == 0) ||
            (color == PieceColor::Black && boardPosition.y == 7);
    }

    std::unique_ptr<Piece> Clone() const override {
        return std::make_unique<Pawn>(*this);
    }
};

class PieceFactory {
public:
    static std::unique_ptr<Piece> CreatePiece(PieceType type, PieceColor color, Vector2Int pos) {
        switch (type) {
        case PieceType::Rook:
            return std::make_unique<Rook>(color, pos);
        case PieceType::Knight:
            return std::make_unique<Knight>(color, pos);
        case PieceType::Bishop:
            return std::make_unique<Bishop>(color, pos);
        case PieceType::Queen:
            return std::make_unique<Queen>(color, pos);
        case PieceType::King:
            return std::make_unique<King>(color, pos);
        case PieceType::Pawn:
            return std::make_unique<Pawn>(color, pos);
        default:
            return nullptr;
        }
    }
};

class Board {
public:
    enum class MoveResult {
        Invalid,
        Success,
        Check,
        Checkmate,
        Stalemate
    };

    
    class MoveCommand : public Command {
    private:
        Board& board;
        Vector2Int from;
        Vector2Int to;
        std::unique_ptr<Piece> movedPiece;
        std::unique_ptr<Piece> capturedPiece;
        bool wasMoved;
        bool promotionOccurred;
        PieceColor previousTurn;

    public:
        MoveCommand(Board& b, Vector2Int f, Vector2Int t,
            std::unique_ptr<Piece> moved, std::unique_ptr<Piece> captured,
            bool movedStatus, bool promoted, PieceColor turn)
            : board(b), from(f), to(t), movedPiece(std::move(moved)),
            capturedPiece(std::move(captured)), wasMoved(movedStatus),
            promotionOccurred(promoted), previousTurn(turn) {}

        void Execute() override {
            
        }

        void Undo() override {
            
            board.squares[from.y][from.x] = std::move(movedPiece);
            board.squares[from.y][from.x]->boardPosition = from;
            board.squares[from.y][from.x]->hasMoved = wasMoved;

            
            board.squares[to.y][to.x] = std::move(capturedPiece);
            if (board.squares[to.y][to.x]) {
                board.squares[to.y][to.x]->boardPosition = to;
            }

            
            board.currentTurn = previousTurn;
            board.gameOver = false;
            board.winner = PieceColor::None;
        }
    };

    std::vector<std::vector<std::unique_ptr<Piece>>> squares;
    PieceColor currentTurn;
    bool gameOver;
    PieceColor winner;
    std::stack<std::unique_ptr<Command>> history;

    Board() : currentTurn(PieceColor::White), gameOver(false), winner(PieceColor::None) {
        squares.resize(BOARD_SIZE);
        for (auto& row : squares) {
            row.resize(BOARD_SIZE);
        }
    }

    Board(const Board&) = delete;
    Board& operator=(const Board&) = delete;

    void Initialize() {
        

        
        for (int i = 0; i < BOARD_SIZE; ++i) {
            squares[1][i] = PieceFactory::CreatePiece(PieceType::Pawn, PieceColor::Black, Vector2Int(i, 1));
        }
        squares[0][0] = PieceFactory::CreatePiece(PieceType::Rook, PieceColor::Black, Vector2Int(0, 0));
        squares[0][1] = PieceFactory::CreatePiece(PieceType::Knight, PieceColor::Black, Vector2Int(1, 0));
        squares[0][2] = PieceFactory::CreatePiece(PieceType::Bishop, PieceColor::Black, Vector2Int(2, 0));
        squares[0][3] = PieceFactory::CreatePiece(PieceType::Queen, PieceColor::Black, Vector2Int(3, 0));
        squares[0][4] = PieceFactory::CreatePiece(PieceType::King, PieceColor::Black, Vector2Int(4, 0));
        squares[0][5] = PieceFactory::CreatePiece(PieceType::Bishop, PieceColor::Black, Vector2Int(5, 0));
        squares[0][6] = PieceFactory::CreatePiece(PieceType::Knight, PieceColor::Black, Vector2Int(6, 0));
        squares[0][7] = PieceFactory::CreatePiece(PieceType::Rook, PieceColor::Black, Vector2Int(7, 0));

        
        for (int i = 0; i < BOARD_SIZE; ++i) {
            squares[6][i] = PieceFactory::CreatePiece(PieceType::Pawn, PieceColor::White, Vector2Int(i, 6));
        }
        squares[7][0] = PieceFactory::CreatePiece(PieceType::Rook, PieceColor::White, Vector2Int(0, 7));
        squares[7][1] = PieceFactory::CreatePiece(PieceType::Knight, PieceColor::White, Vector2Int(1, 7));
        squares[7][2] = PieceFactory::CreatePiece(PieceType::Bishop, PieceColor::White, Vector2Int(2, 7));
        squares[7][3] = PieceFactory::CreatePiece(PieceType::Queen, PieceColor::White, Vector2Int(3, 7));
        squares[7][4] = PieceFactory::CreatePiece(PieceType::King, PieceColor::White, Vector2Int(4, 7));
        squares[7][5] = PieceFactory::CreatePiece(PieceType::Bishop, PieceColor::White, Vector2Int(5, 7));
        squares[7][6] = PieceFactory::CreatePiece(PieceType::Knight, PieceColor::White, Vector2Int(6, 7));
        squares[7][7] = PieceFactory::CreatePiece(PieceType::Rook, PieceColor::White, Vector2Int(7, 7));
    }

    Vector2Int FindKing(PieceColor color) const {
        for (int y = 0; y < BOARD_SIZE; y++) {
            for (int x = 0; x < BOARD_SIZE; x++) {
                if (squares[y][x] && squares[y][x]->type == PieceType::King &&
                    squares[y][x]->color == color) {
                    return Vector2Int(x, y);
                }
            }
        }
        return Vector2Int(-1, -1); 
    }

    bool IsInCheck(PieceColor color) const {
        Vector2Int kingPos = FindKing(color);
        if (kingPos.x == -1) return false;

        
        for (int y = 0; y < BOARD_SIZE; y++) {
            for (int x = 0; x < BOARD_SIZE; x++) {
                if (squares[y][x] && squares[y][x]->color != color) {
                    std::vector<Vector2Int> moves = squares[y][x]->GetValidMoves(squares);
                    for (const auto& move : moves) {
                        if (move == kingPos) {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    bool HasLegalMoves(PieceColor color) {
        for (int y = 0; y < BOARD_SIZE; y++) {
            for (int x = 0; x < BOARD_SIZE; x++) {
                if (squares[y][x] && squares[y][x]->color == color) {
                    std::vector<Vector2Int> moves = squares[y][x]->GetValidMoves(squares);
                    for (const auto& move : moves) {
                        
                        std::unique_ptr<Piece> originalTarget = std::move(squares[move.y][move.x]);
                        Vector2Int originalPos = squares[y][x]->boardPosition;
                        squares[y][x]->boardPosition = move;
                        squares[move.y][move.x] = std::move(squares[y][x]);
                        squares[y][x] = nullptr;

                        
                        bool stillInCheck = IsInCheck(color);

                        
                        squares[y][x] = std::move(squares[move.y][move.x]);
                        squares[y][x]->boardPosition = originalPos;
                        squares[move.y][move.x] = std::move(originalTarget);

                        if (!stillInCheck) {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    MoveResult MovePiece(Vector2Int from, Vector2Int to) {
        if (gameOver) return MoveResult::Invalid;
        if (!Piece::InBounds(from.x, from.y) || !Piece::InBounds(to.x, to.y))
            return MoveResult::Invalid;
        if (!squares[from.y][from.x]) return MoveResult::Invalid;
        auto& piece = squares[from.y][from.x];
        if (piece->color != currentTurn) return MoveResult::Invalid;

        
        std::vector<Vector2Int> validMoves = piece->GetValidMoves(squares);
        bool moveLegal = false;
        for (auto& m : validMoves) {
            if (m == to) {
                moveLegal = true;
                break;
            }
        }
        if (!moveLegal) return MoveResult::Invalid;

        
        bool wasMoved = piece->hasMoved;
        std::unique_ptr<Piece> movedPieceCopy = piece->Clone();
        std::unique_ptr<Piece> capturedPiece = nullptr;
        if (squares[to.y][to.x]) {
            capturedPiece = squares[to.y][to.x]->Clone();
        }
        PieceColor previousTurn = currentTurn;

        
        std::unique_ptr<Piece> originalTarget = std::move(squares[to.y][to.x]);
        Vector2Int originalPos = piece->boardPosition;

        piece->boardPosition = to;
        piece->hasMoved = true;
        bool isPromotion = piece->IsPromotion();

        squares[to.y][to.x] = std::move(squares[from.y][from.x]);
        squares[from.y][from.x] = nullptr;

        
        if (isPromotion) {
            HandlePromotion(to);
        }

        
        if (IsInCheck(previousTurn)) {
            
            squares[from.y][from.x] = std::move(squares[to.y][to.x]);
            squares[from.y][from.x]->boardPosition = originalPos;
            squares[to.y][to.x] = std::move(originalTarget);
            return MoveResult::Invalid;
        }

        
        currentTurn = (currentTurn == PieceColor::White) ? PieceColor::Black : PieceColor::White;

        
        bool promotionOccurred = isPromotion;
        history.push(std::make_unique<MoveCommand>(*this, from, to,
            std::move(movedPieceCopy),
            std::move(capturedPiece),
            wasMoved, promotionOccurred, previousTurn));

        
        bool inCheck = IsInCheck(currentTurn);
        bool hasMoves = HasLegalMoves(currentTurn);

        if (inCheck && !hasMoves) {
            gameOver = true;
            winner = (currentTurn == PieceColor::White) ? PieceColor::Black : PieceColor::White;
            return MoveResult::Checkmate;
        }
        else if (!inCheck && !hasMoves) {
            gameOver = true;
            winner = PieceColor::None;
            return MoveResult::Stalemate;
        }
        else if (inCheck) {
            return MoveResult::Check;
        }

        return MoveResult::Success;
    }

    bool UndoLastMove() {
        if (history.empty()) {
            return false;
        }

        auto& command = history.top();
        command->Undo();
        history.pop();

        return true;
    }

    void HandlePromotion(Vector2Int pos) {
        squares[pos.y][pos.x] = PieceFactory::CreatePiece(PieceType::Queen, squares[pos.y][pos.x]->color, pos);
    }

    Piece* GetPieceAt(const Vector2Int& pos) const {
        if (!Piece::InBounds(pos.x, pos.y)) return nullptr;
        return squares[pos.y][pos.x].get();
    }
};

class ChessGame {
private:
    Board board;
    Texture2D spriteSheet;
    Vector2Int selectedSquare;
    bool pieceSelected;
    std::string statusMessage;

public:
    ChessGame() : selectedSquare(-1, -1), pieceSelected(false), statusMessage("") {}

    void Init() {
        board.Initialize();

        
        Image spriteImage = LoadImage("chess_pieces.png");
        if (spriteImage.data != nullptr) {
            spriteSheet = LoadTextureFromImage(spriteImage);
            UnloadImage(spriteImage);
        }
        else {
            
            TraceLog(LOG_WARNING, "Failed to load chess pieces image!");
        }
    }

    void Update() {
        statusMessage = "";

        
        if (board.gameOver) {
            if (board.winner == PieceColor::None) {
                statusMessage = "Stalemate! Game ended in a draw.";
            }
            else {
                statusMessage = (board.winner == PieceColor::White)
                    ? "Checkmate! White wins!"
                    : "Checkmate! Black wins!";
            }
            return;
        }

        
        if (IsKeyPressed(KEY_U)) {
            if (board.UndoLastMove()) {
                statusMessage = "Undo successful!";
                pieceSelected = false;
                selectedSquare = Vector2Int(-1, -1);
            }
            else {
                statusMessage = "No moves to undo!";
            }
            return;
        }

        
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            int mx = GetMouseX();
            int my = GetMouseY();
            int col = mx / TILE_SIZE;
            int row = my / TILE_SIZE;
            Vector2Int clickedPos(col, row);

            if (!pieceSelected) {
                
                Piece* p = board.GetPieceAt(clickedPos);
                if (p && p->color == board.currentTurn) {
                    selectedSquare = clickedPos;
                    pieceSelected = true;
                }
            }
            else {
                
                Board::MoveResult result = board.MovePiece(selectedSquare, clickedPos);

                if (result == Board::MoveResult::Success ||
                    result == Board::MoveResult::Check ||
                    result == Board::MoveResult::Checkmate) {

                    
                    pieceSelected = false;
                    selectedSquare = Vector2Int(-1, -1);

                    
                    if (result == Board::MoveResult::Check) {
                        statusMessage = (board.currentTurn == PieceColor::White)
                            ? "White is in check!"
                            : "Black is in check!";
                    }
                }
                else {
                    
                    if (clickedPos == selectedSquare) {
                        
                        pieceSelected = false;
                        selectedSquare = Vector2Int(-1, -1);
                    }
                    else {
                        
                        Piece* p = board.GetPieceAt(clickedPos);
                        if (p && p->color == board.currentTurn) {
                            selectedSquare = clickedPos;
                        }
                    }
                }
            }
        }
    }

    void Draw() {
        ClearBackground(RAYWHITE);
        DrawBoard();
        DrawPieces();

        
        if (pieceSelected) {
            Rectangle highlight = {
                (float)(selectedSquare.x * TILE_SIZE),
                (float)(selectedSquare.y * TILE_SIZE),
                (float)TILE_SIZE,
                (float)TILE_SIZE
            };
            DrawRectangleLinesEx(highlight, 4, GREEN);
        }

        
        std::string turnText = (board.currentTurn == PieceColor::White)
            ? "Turn: White"
            : "Turn: Black";
        DrawText(turnText.c_str(), 10, BOARD_SIZE * TILE_SIZE + 10, 20, BLACK);

        
        if (!statusMessage.empty()) {
            int textWidth = MeasureText(statusMessage.c_str(), 30);
            DrawText(statusMessage.c_str(),
                BOARD_SIZE * TILE_SIZE / 2 - textWidth / 2,
                BOARD_SIZE * TILE_SIZE + 40, 30, RED);
        }

        
        std::string undoHint = "Press 'U' to Undo";
        int hintWidth = MeasureText(undoHint.c_str(), 20);
        DrawText(undoHint.c_str(),
            BOARD_SIZE * TILE_SIZE - hintWidth - 10,
            BOARD_SIZE * TILE_SIZE + 10, 20, DARKGRAY);
    }

    void DrawBoard() {
        Color light = { 240, 217, 181, 255 };    
        Color dark = { 181, 136, 99, 255 };       

        for (int y = 0; y < BOARD_SIZE; ++y) {
            for (int x = 0; x < BOARD_SIZE; ++x) {
                bool isLight = (x + y) % 2 == 0;
                DrawRectangle(x * TILE_SIZE, y * TILE_SIZE,
                    TILE_SIZE, TILE_SIZE,
                    isLight ? light : dark);
            }
        }
    }

    void DrawPieces() {
        const int pieceWidth = 56;   
        const int pieceHeight = 60;  

        for (int y = 0; y < BOARD_SIZE; ++y) {
            for (int x = 0; x < BOARD_SIZE; ++x) {
                const auto& piecePtr = board.squares[y][x];
                if (piecePtr) {
                    PieceColor c = piecePtr->color;
                    PieceType t = piecePtr->type;

                    
                    int pieceIndex = 0;
                    switch (t) {
                    case PieceType::Rook: pieceIndex = 0; break;
                    case PieceType::Knight: pieceIndex = 1; break;
                    case PieceType::Bishop: pieceIndex = 2; break;
                    case PieceType::Queen: pieceIndex = 3; break;
                    case PieceType::King: pieceIndex = 4; break;
                    case PieceType::Pawn: pieceIndex = 5; break;
                    default: pieceIndex = 0; break;
                    }

                    
                    int row = (c == PieceColor::Black) ? 0 : 1;

                    
                    Rectangle sourceRec = {
                        pieceIndex * pieceWidth * 1.0f,
                        row * pieceHeight * 1.0f,
                        pieceWidth * 1.0f,
                        pieceHeight * 1.0f
                    };

                    
                    Rectangle destRec = {
                        x * TILE_SIZE + (TILE_SIZE - pieceWidth) / 2.0f,
                        y * TILE_SIZE + (TILE_SIZE - pieceHeight) / 2.0f,
                        (float)pieceWidth,
                        (float)pieceHeight
                    };

                    
                    DrawTexturePro(spriteSheet, sourceRec, destRec, { 0,0 }, 0.0f, WHITE);
                }
            }
        }
    }

    void Close() {
        UnloadTexture(spriteSheet);
    }
};

int main() {
    const int screenWidth = BOARD_SIZE * TILE_SIZE;
    const int screenHeight = BOARD_SIZE * TILE_SIZE + 100; 

    InitWindow(screenWidth, screenHeight, "Chess Game with raylib");
    SetTargetFPS(60);

    ChessGame game;
    game.Init();

    while (!WindowShouldClose()) {
        game.Update();

        BeginDrawing();
        game.Draw();
        EndDrawing();
    }

    game.Close();
    CloseWindow();

    return 0;
}