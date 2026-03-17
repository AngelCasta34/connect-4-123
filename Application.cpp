#include "Application.h"
#include "imgui/imgui.h"
#include "classes/TicTacToe.h"
#include "classes/Checkers.h"
#include "classes/Othello.h"
#include "classes/Connect4.h"


namespace ClassGame {
        //
        // our global variables
        //
        Game *game = nullptr;
        bool gameOver = false;
        int gameWinner = -1;

        //
        // game starting point
        // this is called by the main render loop in main.cpp
        //
        void GameStartUp() 
        {
            game = nullptr;
        }

        //
        // game render loop
        // this is called by the main render loop in main.cpp
        //
        void RenderGame() 
        {
                ImGui::DockSpaceOverViewport();

                //ImGui::ShowDemoWindow();

                ImGui::Begin("Settings");

                if (gameOver) {
                    ImGui::Text("Game Over!");
                    ImGui::Text("Winner: %d", gameWinner);
                    if (ImGui::Button("Reset Game")) {
                        game->stopGame();
                        game->setUpBoard();
                        gameOver = false;
                        gameWinner = -1;
                    }
                }
                if (!game) {
                    if (ImGui::Button("Start Tic-Tac-Toe")) {
                        game = new TicTacToe();
                        game->setUpBoard();
                    }
                    if (ImGui::Button("Start Checkers")) {
                        game = new Checkers();
                        game->setUpBoard();
                    }
                    if (ImGui::Button("Start Connect 4")) {
                        game = new Connect4();
                        game->setUpBoard();
                    }
                    if (ImGui::Button("Start Othello")) {
                        game = new Othello();
                        game->setUpBoard();
                    }
                } else {
                    ImGui::Text("Current Player Number: %d", game->getCurrentPlayer()->playerNumber());
                    ImGui::Text("Current Board State: %s", game->stateString().c_str());

                    if (game->gameHasAI()) {
                        static int aiPlayerChoice = 1; // 0 = player 1 (Red), 1 = player 2 (Yellow)
                        bool aiEnabled = game->_gameOptions.AIPlaying;

                        ImGui::Text("AI plays as:");
                        ImGui::SameLine();
                        if (ImGui::RadioButton("Player 1 (Red)", &aiPlayerChoice, 0)) {
                            if (aiEnabled) game->setAIPlaying(true, 0);
                        }
                        ImGui::SameLine();
                        if (ImGui::RadioButton("Player 2 (Yellow)", &aiPlayerChoice, 1)) {
                            if (aiEnabled) game->setAIPlaying(true, 1);
                        }

                        if (ImGui::Checkbox("Play vs AI", &aiEnabled)) {
                            game->setAIPlaying(aiEnabled, aiPlayerChoice);
                        }

                        if (game->_gameOptions.AIPlaying) {
                            ImGui::Text("AI is Player %d", game->getAIPlayer() + 1);
                        }
                    }
                }
                ImGui::End();

                ImGui::Begin("GameWindow");
                if (game) {
                    if (game->_gameOptions.AIPlaying &&
                        game->gameHasAI() &&
                        (game->getCurrentPlayer()->isAIPlayer() || game->_gameOptions.AIvsAI))
                    {
                        game->updateAI();
                    }
                    game->drawFrame();
                }
                ImGui::End();
        }

        //
        // end turn is called by the game code at the end of each turn
        // this is where we check for a winner
        //
        void EndOfTurn() 
        {
            Player *winner = game->checkForWinner();
            if (winner)
            {
                gameOver = true;
                gameWinner = winner->playerNumber();
            }
            if (game->checkForDraw()) {
                gameOver = true;
                gameWinner = -1;
            }
        }
}
// EOF 