#include "Connect4.h"
#include <array>

Connect4::Connect4()
{
    _grid = new Grid(COLS, ROWS);
}

Connect4::~Connect4()
{
    delete _grid;
}

Bit* Connect4::pieceForPlayer(int playerNumber)
{
    // playerNumber is 0 or 1 (matches Game's players vector)
    Bit* bit = new Bit();
    bit->LoadTextureFromFile(playerNumber == 0 ? "red.png" : "yellow.png");
    bit->setOwner(getPlayerAt(playerNumber));
    return bit;
}

void Connect4::setUpBoard()
{
    setNumberOfPlayers(2);

    _gameOptions.rowX = COLS;
    _gameOptions.rowY = ROWS;

    // Match how Checkers sets up the board squares
    _grid->initializeSquares(80, "boardsquare.png");

    // Enable all squares (Grid defaults enabled, but this keeps intent clear)
    _grid->forEachSquare([&](ChessSquare* sq, int x, int y) {
        _grid->setEnabled(x, y, true);
        sq->destroyBit(); // safety if reusing board
    });

    clearAIPlayers();

    startGame();
}

ChessSquare* Connect4::lowestEmptyInColumn(int col)
{
    // bottom row is ROWS-1
    for (int row = ROWS - 1; row >= 0; --row)
    {
        ChessSquare* sq = _grid->getSquare(col, row);
        if (sq && !sq->bit())
            return sq;
    }
    return nullptr;
}

bool Connect4::actionForEmptyHolder(BitHolder& holder)
{
    // We ignore the clicked row and treat clicks as "choose this column"
    ChessSquare* clicked = static_cast<ChessSquare*>(&holder);
    int col = clicked->getColumn();

    ChessSquare* target = lowestEmptyInColumn(col);
    if (!target) return false; // column full

    if (target->bit()) return false; // should never happen, but safe

    Bit* bit = pieceForPlayer(getCurrentPlayer()->playerNumber());
    // Start piece one square above the board, animate it falling down
    ChessSquare* topSquare = _grid->getSquare(col, 0);
    bit->setPosition(ImVec2(target->getPosition().x, topSquare->getPosition().y - 80.0f));
    target->setBit(bit);
    bit->moveTo(target->getPosition());

    endTurn();
    return true;
}

bool Connect4::canBitMoveFrom(Bit& bit, BitHolder& src)
{
    // No dragging/moving pieces in Connect 4
    return false;
}

bool Connect4::canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst)
{
    // No dragging/moving pieces in Connect 4
    return false;
}

void Connect4::stopGame()
{
    // Required: clean up any heap memory you allocated (the Bits)
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->destroyBit();
    });
}

Player* Connect4::ownerAt(int x, int y) const
{
    ChessSquare* sq = _grid->getSquare(x, y);
    if (!sq || !sq->bit()) return nullptr;
    return sq->bit()->getOwner();
}

bool Connect4::fourInARowFrom(int x, int y, int dx, int dy, Player* p) const
{
    // Check 4 cells: (x,y) (x+dx,y+dy) (x+2dx,y+2dy) (x+3dx,y+3dy)
    for (int i = 0; i < 4; ++i)
    {
        int xx = x + dx * i;
        int yy = y + dy * i;
        if (xx < 0 || xx >= COLS || yy < 0 || yy >= ROWS) return false;
        if (ownerAt(xx, yy) != p) return false;
    }
    return true;
}

Player* Connect4::checkForWinner()
{
    // Scan all possible 4-in-a-row starts
    for (int y = 0; y < ROWS; ++y)
    {
        for (int x = 0; x < COLS; ++x)
        {
            Player* p = ownerAt(x, y);
            if (!p) continue;

            // Horizontal →
            if (x <= COLS - 4 && fourInARowFrom(x, y, 1, 0, p)) return p;
            // Vertical ↓
            if (y <= ROWS - 4 && fourInARowFrom(x, y, 0, 1, p)) return p;
            // Diagonal down-right ↘
            if (x <= COLS - 4 && y <= ROWS - 4 && fourInARowFrom(x, y, 1, 1, p)) return p;
            // Diagonal down-left ↙
            if (x >= 3 && y <= ROWS - 4 && fourInARowFrom(x, y, -1, 1, p)) return p;
        }
    }
    return nullptr;
}

bool Connect4::checkForDraw()
{
    // Draw if board is full and no winner
    bool full = true;
    for (int x = 0; x < COLS; ++x)
    {
        ChessSquare* top = _grid->getSquare(x, 0);
        if (top && !top->bit())
        {
            full = false;
            break;
        }
    }
    return full && (checkForWinner() == nullptr);
}

std::string Connect4::initialStateString()
{
    // 42 cells: 0 empty, 1 player0, 2 player1
    return std::string(ROWS * COLS, '0');
}

std::string Connect4::stateString()
{
    std::string s(ROWS * COLS, '0');
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        Bit* bit = square->bit();
        if (bit)
        {
            // Store 1 or 2
            s[y * COLS + x] = char('0' + (bit->getOwner()->playerNumber() + 1));
        }
    });
    return s;
}

void Connect4::setStateString(const std::string& s)
{
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        int idx = y * COLS + x;
        int v = s[idx] - '0';
        if (v == 0)
        {
            square->destroyBit();
        }
        else
        {
            square->destroyBit();
            Bit* bit = pieceForPlayer(v - 1);
            bit->setPosition(square->getPosition());
            square->setBit(bit);
        }
    });
}

int Connect4::firstValidMovePreferCenter()
{
    std::array<int, COLS> order = {3, 2, 4, 1, 5, 0, 6};
    for (int col : order)
    {
        if (lowestEmptyInColumn(col))
            return col;
    }
    return -1;
}

//Negamax helpers 

Connect4::Board Connect4::getBoardState() const
{
    Board board = {};
    for (int y = 0; y < ROWS; ++y)
        for (int x = 0; x < COLS; ++x)
        {
            Player* p = ownerAt(x, y);
            if (p) board[y][x] = p->playerNumber() + 1; // 1 or 2
        }
    return board;
}

int Connect4::dropOnBoard(Board& board, int col, int playerIdx) const
{
    for (int row = ROWS - 1; row >= 0; --row)
    {
        if (board[row][col] == 0)
        {
            board[row][col] = playerIdx + 1;
            return row;
        }
    }
    return -1; // column full
}

bool Connect4::checkWinOnBoard(const Board& board, int playerIdx) const
{
    int p = playerIdx + 1;
    // Horizontal
    for (int y = 0; y < ROWS; ++y)
        for (int x = 0; x <= COLS - 4; ++x)
            if (board[y][x]==p && board[y][x+1]==p && board[y][x+2]==p && board[y][x+3]==p)
                return true;
    // Vertical
    for (int x = 0; x < COLS; ++x)
        for (int y = 0; y <= ROWS - 4; ++y)
            if (board[y][x]==p && board[y+1][x]==p && board[y+2][x]==p && board[y+3][x]==p)
                return true;
    // Diagonal down-right
    for (int y = 0; y <= ROWS - 4; ++y)
        for (int x = 0; x <= COLS - 4; ++x)
            if (board[y][x]==p && board[y+1][x+1]==p && board[y+2][x+2]==p && board[y+3][x+3]==p)
                return true;
    // Diagonal down-left
    for (int y = 0; y <= ROWS - 4; ++y)
        for (int x = 3; x < COLS; ++x)
            if (board[y][x]==p && board[y+1][x-1]==p && board[y+2][x-2]==p && board[y+3][x-3]==p)
                return true;
    return false;
}

int Connect4::scoreWindow(int a, int b, int c, int d, int playerIdx) const
{
    int p   = playerIdx + 1;
    int opp = (playerIdx == 0) ? 2 : 1;
    int pieces = 0, empty = 0, oppPieces = 0;
    for (int v : {a, b, c, d})
    {
        if      (v == p)   ++pieces;
        else if (v == opp) ++oppPieces;
        else               ++empty;
    }
    if (pieces == 4)                    return  100;
    if (pieces == 3 && empty == 1)      return    5;
    if (pieces == 2 && empty == 2)      return    2;
    if (oppPieces == 3 && empty == 1)   return   -4;
    return 0;
}

int Connect4::evaluateBoard(const Board& board, int playerIdx) const
{
    int score = 0;
    int p = playerIdx + 1;

    // Center column preference
    for (int y = 0; y < ROWS; ++y)
        if (board[y][COLS / 2] == p) score += 3;

    // Horizontal windows
    for (int y = 0; y < ROWS; ++y)
        for (int x = 0; x <= COLS - 4; ++x)
            score += scoreWindow(board[y][x], board[y][x+1], board[y][x+2], board[y][x+3], playerIdx);

    // Vertical windows
    for (int x = 0; x < COLS; ++x)
        for (int y = 0; y <= ROWS - 4; ++y)
            score += scoreWindow(board[y][x], board[y+1][x], board[y+2][x], board[y+3][x], playerIdx);

    // Diagonal down-right
    for (int y = 0; y <= ROWS - 4; ++y)
        for (int x = 0; x <= COLS - 4; ++x)
            score += scoreWindow(board[y][x], board[y+1][x+1], board[y+2][x+2], board[y+3][x+3], playerIdx);

    // Diagonal down-left
    for (int y = 0; y <= ROWS - 4; ++y)
        for (int x = 3; x < COLS; ++x)
            score += scoreWindow(board[y][x], board[y+1][x-1], board[y+2][x-2], board[y+3][x-3], playerIdx);

    return score;
}

// Returns score from currentPlayer's 
int Connect4::negamax(Board& board, int depth, int alpha, int beta, int playerIdx) const
{
    int oppIdx = 1 - playerIdx;

    if (checkWinOnBoard(board, oppIdx))
        return -100000 - depth; 

    // Check draw 
    bool full = true;
    for (int x = 0; x < COLS; ++x)
        if (board[0][x] == 0) { full = false; break; }
    if (full) return 0;

    if (depth == 0)
        return evaluateBoard(board, playerIdx);

    std::array<int, COLS> order = {3, 2, 4, 1, 5, 0, 6};
    int best = -200000;

    for (int col : order)
    {
        int row = dropOnBoard(board, col, playerIdx);
        if (row == -1) continue;

        int score = -negamax(board, depth - 1, -beta, -alpha, oppIdx);
        board[row][col] = 0; // undo

        if (score > best)  best = score;
        if (score > alpha) alpha = score;
        if (alpha >= beta) break; // prune
    }
    return best;
}

void Connect4::updateAI()
{
    Player* ai = getPlayerAt(getAIPlayer());
    int aiIdx  = ai->playerNumber(); // 0 or 1
    int oppIdx = 1 - aiIdx;

    Board board = getBoardState();

    int bestScore = -200000;
    int bestCol   = firstValidMovePreferCenter();

    std::array<int, COLS> order = {3, 2, 4, 1, 5, 0, 6};
    for (int col : order)
    {
        int row = dropOnBoard(board, col, aiIdx);
        if (row == -1) continue;

        int score = -negamax(board, 6, -200000, 200000, oppIdx);
        board[row][col] = 0;

        if (score > bestScore)
        {
            bestScore = score;
            bestCol   = col;
        }
    }

    if (bestCol == -1) return;

    ChessSquare* target = lowestEmptyInColumn(bestCol);
    if (!target) return;

    Bit* bit = pieceForPlayer(ai->playerNumber());
    // Animate piece dropping from above
    ChessSquare* topSquare = _grid->getSquare(bestCol, 0);
    bit->setPosition(ImVec2(target->getPosition().x, topSquare->getPosition().y - 80.0f));
    target->setBit(bit);
    bit->moveTo(target->getPosition());

    endTurn();
}
