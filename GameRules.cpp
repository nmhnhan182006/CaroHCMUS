#include "GameRules.h"
#include "GameCore.h"

namespace caro {

    namespace {

        int CountOneDirection(
            const GameSession& game,
            Position start,
            CellState symbol,
            int dRow,
            int dCol
        ) {
            int count = 0;
            int r = start.row + dRow;
            int c = start.col + dCol;
            int n = (int)game.board.size();

            while (r >= 0 && r < n && c >= 0 && c < n && game.board[r][c] == symbol) {
                ++count;
                r += dRow;
                c += dCol;
            }

            return count;
        }

        bool IsBlockedEnd(
            const GameSession& game,
            int row,
            int col,
            CellState symbol
        ) {
            int n = (int)game.board.size();

            if (row < 0 || row >= n || col < 0 || col >= n) {
                return true;
            }

            CellState value = game.board[row][col];
            return (value != CellState::Empty && value != symbol);
        }

        bool CheckDirectionStandard(
            const GameSession& game,
            Position lastMove,
            CellState symbol,
            int dRow,
            int dCol
        ) {
            int backCount = 0;
            int r = lastMove.row - dRow;
            int c = lastMove.col - dCol;
            int n = (int)game.board.size();

            while (r >= 0 && r < n && c >= 0 && c < n && game.board[r][c] == symbol) {
                ++backCount;
                r -= dRow;
                c -= dCol;
            }

            int end1Row = r;
            int end1Col = c;

            int forwardCount = 0;
            r = lastMove.row + dRow;
            c = lastMove.col + dCol;

            while (r >= 0 && r < n && c >= 0 && c < n && game.board[r][c] == symbol) {
                ++forwardCount;
                r += dRow;
                c += dCol;
            }

            int end2Row = r;
            int end2Col = c;

            int total = backCount + 1 + forwardCount;
            if (total < config::WIN_LENGTH) {
                return false;
            }

            bool blocked1 = IsBlockedEnd(game, end1Row, end1Col, symbol);
            bool blocked2 = IsBlockedEnd(game, end2Row, end2Col, symbol);

            return !(blocked1 && blocked2);
        }

        GameResult ResultFromCell(CellState symbol) {
            if (symbol == CellState::X) return GameResult::XWin;
            if (symbol == CellState::O) return GameResult::OWin;
            return GameResult::InProgress;
        }

    } // namespace

    int CountContinuousCells(
        const GameSession& game,
        Position start,
        CellState symbol,
        int dRow,
        int dCol
    ) {
        if (!IsInsideBoard(game, start)) return 0;
        if (GetCell(game, start) != symbol) return 0;

        int total = 1;
        total += CountOneDirection(game, start, symbol, dRow, dCol);
        total += CountOneDirection(game, start, symbol, -dRow, -dCol);
        return total;
    }

    bool HasFiveInRow(
        const GameSession& game,
        Position lastMove,
        CellState symbol
    ) {
        static const int directions[4][2] = {
            {0, 1}, {1, 0}, {1, 1}, {1, -1}
        };

        for (int i = 0; i < 4; ++i) {
            int total = CountContinuousCells(
                game,
                lastMove,
                symbol,
                directions[i][0],
                directions[i][1]
            );

            if (total >= config::WIN_LENGTH) {
                return true;
            }
        }

        return false;
    }

    bool IsStandardRuleWinningLine(
        const GameSession& game,
        Position lastMove,
        CellState symbol
    ) {
        static const int directions[4][2] = {
            {0, 1}, {1, 0}, {1, 1}, {1, -1}
        };

        for (int i = 0; i < 4; ++i) {
            if (CheckDirectionStandard(game, lastMove, symbol, directions[i][0], directions[i][1])) {
                return true;
            }
        }

        return false;
    }

    GameResult EvaluateBoard(const GameSession& game, Position lastMove) {
        if (!IsInsideBoard(game, lastMove)) {
            return GameResult::InProgress;
        }

        CellState symbol = GetCell(game, lastMove);
        if (symbol == CellState::Empty) {
            return IsBoardFull(game) ? GameResult::Draw : GameResult::InProgress;
        }

        bool isWin = false;

        if (game.settings.ruleMode == RuleMode::FreeStyle) {
            isWin = HasFiveInRow(game, lastMove, symbol);
        }
        else {
            isWin = IsStandardRuleWinningLine(game, lastMove, symbol);
        }

        if (isWin) {
            return ResultFromCell(symbol);
        }

        if (IsBoardFull(game)) {
            return GameResult::Draw;
        }

        return GameResult::InProgress;
    }

} // namespace caro