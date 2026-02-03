#pragma once
#include "Game.h"

class Connect4 : public Game
{
public:
    Connect4();
    ~Connect4();

    void        setUpBoard() override;

    Player*     checkForWinner() override;
    bool        checkForDraw() override;

    std::string initialStateString() override;
    std::string stateString() override;
    void        setStateString(const std::string& s) override;

    bool        actionForEmptyHolder(BitHolder& holder) override;
    bool        canBitMoveFrom(Bit& bit, BitHolder& src) override;
    bool        canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst) override;

    void        stopGame() override;

    void        updateAI() override;
    bool        gameHasAI() override { return true; }

    Grid*       getGrid() override { return _grid; }

private:
    static constexpr int ROWS = 6;
    static constexpr int COLS = 7;

    Grid* _grid;

    Bit* pieceForPlayer(int playerNumber); // 0 = red, 1 = yellow

    // Drop logic
    ChessSquare* lowestEmptyInColumn(int col);

    // Win helpers
    Player* ownerAt(int x, int y) const;
    bool fourInARowFrom(int x, int y, int dx, int dy, Player* p) const;

    // AI helpers (simple, good enough for grading)
    int findWinningMoveFor(Player* p);
    int firstValidMovePreferCenter();
};
