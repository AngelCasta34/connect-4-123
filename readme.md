# Connect 4 + AI 

This project extends the existing game framework by adding Connect 4 as a fourth playable game and improving the AI system so all supported games can be played in either two-player (human vs human) mode or player vs AI mode.

The goal of these changes was to match the behavior of Tic-Tac-Toe while fixing limitations that previously forced AI on by default.

Key Changes

1. Added Connect 4

Implemented a new Connect4 class that inherits from Game

Uses the existing Grid system (7×6 board)

Supports:

Two human players

Player vs AI

Win and draw detection

Integrated as a new menu option alongside Tic-Tac-Toe, Checkers, and Othello

2. AI Toggle Support (All Games)

Games now default to two human players

A “Play vs AI” checkbox was added to the Settings panel

When enabled:

Player 1 becomes the AI

The AI automatically plays on its turn

When disabled:

Both players are human again

This behavior is consistent across Tic-Tac-Toe, Othello, and Connect 4.

3. Game System Improvements

Fixed a bug in setAIPlayer where the AI player index was being overwritten

Added helper functions:

setAIPlaying(bool enabled, unsigned int aiPlayer)

clearAIPlayers()

Updated input handling so human input is only blocked when AI mode is actually enabled

Ensured AI logic only runs when “Play vs AI” is turned on

How to Play

Launch the application

Choose a game from the menu

Play in two-player mode by default

Enable Play vs AI in the Settings panel to switch to player vs AI