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
    bit->setPosition(target->getPosition());
    target->setBit(bit);

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
    // Good default for Connect 4: prefer center columns
    std::array<int, COLS> order = {3, 2, 4, 1, 5, 0, 6};
    for (int col : order)
    {
        if (lowestEmptyInColumn(col))
            return col;
    }
    return -1;
}

int Connect4::findWinningMoveFor(Player* p)
{
    // Try each column: if placing a piece there makes p win, return that col
    for (int col = 0; col < COLS; ++col)
    {
        ChessSquare* target = lowestEmptyInColumn(col);
        if (!target) continue;

        // Temporarily place a fake piece by creating a Bit on the square
        Bit* temp = new Bit();
        temp->LoadTextureFromFile("square.png"); // texture does not matter for logic
        temp->setOwner(p);

        target->setBit(temp);

        Player* w = checkForWinner();

        // Undo
        target->destroyBit();

        if (w == p)
            return col;
    }
    return -1;
}

void Connect4::updateAI()
{
    // AI player is the Game player at index _gameOptions.AIPlayer
    Player* ai = getPlayerAt(getAIPlayer());
    Player* human = getPlayerAt(getHumanPlayer());

    // 1) Win if possible
    int col = findWinningMoveFor(ai);

    // 2) Block if human can win next
    if (col == -1)
        col = findWinningMoveFor(human);

    // 3) Otherwise pick a reasonable column
    if (col == -1)
        col = firstValidMovePreferCenter();

    if (col == -1) return;

    ChessSquare* target = lowestEmptyInColumn(col);
    if (!target) return;

    Bit* bit = pieceForPlayer(ai->playerNumber());
    bit->setPosition(target->getPosition());
    target->setBit(bit);

    endTurn();
}
