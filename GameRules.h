#pragma once

#include "Types.h"

namespace caro {

    // =====================================================
    // Kiểm tra luật / kết quả
    // =====================================================

    int CountContinuousCells(
        const GameSession& game,
        Position start,
        CellState symbol,
        int dRow,
        int dCol
    );

    bool HasFiveInRow(
        const GameSession& game,
        Position lastMove,
        CellState symbol
    );

    bool IsStandardRuleWinningLine(
        const GameSession& game,
        Position lastMove,
        CellState symbol
    );

    GameResult EvaluateBoard(const GameSession& game, Position lastMove);

} // namespace caro