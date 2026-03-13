#include "GameAI.h"
#include "GameCore.h"
#include "GameRules.h"

#include <cstdlib>
#include <vector>

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

        GameResult ResultFromCell(CellState symbol) {
            if (symbol == CellState::X) return GameResult::XWin;
            if (symbol == CellState::O) return GameResult::OWin;
            return GameResult::InProgress;
        }

        int EvaluateMoveScore(const GameSession& game, Position pos, CellState symbol) {
            static const int directions[4][2] = {
                {0, 1}, {1, 0}, {1, 1}, {1, -1}
            };

            int score = 0;
            int center = (int)game.board.size() / 2;
            score += 100 - (std::abs(pos.row - center) + std::abs(pos.col - center));

            for (int i = 0; i < 4; ++i) {
                int dRow = directions[i][0];
                int dCol = directions[i][1];

                int own = CountOneDirection(game, pos, symbol, dRow, dCol)
                    + CountOneDirection(game, pos, symbol, -dRow, -dCol);

                int opp = CountOneDirection(game, pos, OpponentOf(symbol), dRow, dCol)
                    + CountOneDirection(game, pos, OpponentOf(symbol), -dRow, -dCol);

                score += own * own * 10;
                score += opp * opp * 8;
            }

            return score;
        }

    } // namespace

    Position FindBestAIMove(const GameSession& game) {
        std::vector<Position> moves = GetAvailableMoves(game);
        if (moves.empty()) return Position(-1, -1);

        CellState me = game.currentTurn;
        CellState opp = OpponentOf(me);

        int n = GetBoardSize(game);
        Position center(n / 2, n / 2);
        if (IsInsideBoard(game, center) && IsCellEmpty(game, center)) {
            return center;
        }

        for (size_t i = 0; i < moves.size(); ++i) {
            GameSession temp = game;
            temp.board[moves[i].row][moves[i].col] = me;
            if (EvaluateBoard(temp, moves[i]) == ResultFromCell(me)) {
                return moves[i];
            }
        }

        for (size_t i = 0; i < moves.size(); ++i) {
            GameSession temp = game;
            temp.board[moves[i].row][moves[i].col] = opp;
            if (EvaluateBoard(temp, moves[i]) == ResultFromCell(opp)) {
                return moves[i];
            }
        }

        int bestScore = -1000000000;
        Position bestMove = moves[0];

        for (size_t i = 0; i < moves.size(); ++i) {
            int score = EvaluateMoveScore(game, moves[i], me);
            if (score > bestScore) {
                bestScore = score;
                bestMove = moves[i];
            }
        }

        return bestMove;
    }

} // namespace caro