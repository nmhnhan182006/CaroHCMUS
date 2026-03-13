#include "GameSession.h"
#include "GameAPI.h"
#include <iostream>

namespace caro {

    void InitializeSession(GameSession& session) {

        session.board.clear();
        session.history.clear();

        session.moveCount = 0;
        session.xMoveCount = 0;
        session.oMoveCount = 0;

        session.xWinCount = 0;
        session.oWinCount = 0;
        session.drawCount = 0;

        session.currentTurn = CellState::X;
        session.result = GameResult::InProgress;

        session.screen = ScreenState::Playing;
        session.isPaused = false;

        session.currentSaveName.clear();
    }

    void PrintSessionInfo(const GameSession& session) {

        std::cout << "Current turn : " << ToString(session.currentTurn) << "\n";
        std::cout << "Result       : " << ToString(session.result) << "\n";

        std::cout << "Move count   : " << session.moveCount << "\n";
        std::cout << "X moves      : " << session.xMoveCount << "\n";
        std::cout << "O moves      : " << session.oMoveCount << "\n";

        std::cout << "X wins       : " << session.xWinCount << "\n";
        std::cout << "O wins       : " << session.oWinCount << "\n";
        std::cout << "Draws        : " << session.drawCount << "\n";
    }

}