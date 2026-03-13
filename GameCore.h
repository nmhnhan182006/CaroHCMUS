#pragma once

#include "Types.h"

namespace caro {

    // =====================================================
    // Khởi tạo / reset
    // =====================================================

    void InitializeEmptyBoard(GameSession& game);

    void StartNewGame(
        GameSession& game,
        const GameSettings& settings,
        const std::string& playerXName = config::DEFAULT_PLAYER_X_NAME,
        const std::string& playerOName = config::DEFAULT_PLAYER_O_NAME
    );

    void ResetCurrentMatch(GameSession& game);

    // =====================================================
    // Truy cập dữ liệu bàn cờ
    // =====================================================

    int GetBoardSize(const GameSession& game);

    bool IsInsideBoard(const GameSession& game, Position pos);

    bool IsCellEmpty(const GameSession& game, Position pos);

    CellState GetCell(const GameSession& game, Position pos);

    const std::vector<std::vector<CellState> >& GetBoard(const GameSession& game);

    // =====================================================
    // Điều khiển lượt chơi
    // =====================================================

    ActionResult PlaceCurrentTurn(GameSession& game, Position pos);

    void SwitchTurn(GameSession& game);

    bool IsBoardFull(const GameSession& game);

    std::vector<Position> GetAvailableMoves(const GameSession& game);

} // namespace caro