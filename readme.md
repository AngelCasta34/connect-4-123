**Features:**

- Two human players (default)
- Player vs AI using negamax with alpha-beta pruning (depth 6)
- AI can play as either Player 1 (Red) or Player 2 (Yellow)
- Win detection: horizontal, vertical, and both diagonals
- Draw detection: board full with no winner
- Animated piece drop — tokens fall from above the board into place

## AI

The Connect 4 AI uses negamax with alpha-beta pruning:

- Searches 6 moves ahead
- Evaluates positions by scoring all 4-cell windows on the board
- Prefers center columns for better positional play
- Columns are searched center-out to improve alpha-beta cutoffs
