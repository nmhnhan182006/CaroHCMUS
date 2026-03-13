#include "GameCore.h"
#include "GameRules.h"

#include <vector>

namespace caro {

    namespace {

        int ClampBoardSize(int value) {
            if (value < config::MIN_BOARD_SIZE) return config::MIN_BOARD_SIZE;
            if (value > config::MAX_BOARD_SIZE) return config::MAX_BOARD_SIZE;
            return value;
        }

    } // namespace

    void InitializeEmptyBoard(GameSession& game) {
        game.settings.boardSize = ClampBoardSize(game.settings.boardSize);

        game.board.assign(
            game.settings.boardSize,
            std::vector<CellState>(game.settings.boardSize, CellState::Empty)
        );

        game.history.clear();
        game.moveCount = 0;
        game.xMoveCount = 0;
        game.oMoveCount = 0;
        game.result = GameResult::InProgress;
        game.currentTurn = CellState::X;
        game.isPaused = false;
    }

    void StartNewGame(
        GameSession& game,
        const GameSettings& settings,
        const std::string& playerXName,
        const std::string& playerOName
    ) {
        game.settings = settings;
        game.settings.boardSize = ClampBoardSize(game.settings.boardSize);

        game.playerX.name = playerXName.empty() ? std::string(config::DEFAULT_PLAYER_X_NAME) : playerXName;
        game.playerX.symbol = CellState::X;
        game.playerX.isBot = false;
        game.playerX.difficulty = AIDifficulty::Easy;

        game.playerO.symbol = CellState::O;
        game.playerO.isBot = (settings.gameMode == GameMode::PVE);
        game.playerO.difficulty = settings.aiDifficulty;

        if (settings.gameMode == GameMode::PVE) {
            if (playerOName.empty() || playerOName == config::DEFAULT_PLAYER_O_NAME) {
                game.playerO.name = config::DEFAULT_BOT_NAME;
            }
            else {
                game.playerO.name = playerOName;
            }
        }
        else {
            game.playerO.name = playerOName.empty() ? std::string(config::DEFAULT_PLAYER_O_NAME) : playerOName;
        }

        game.screen = ScreenState::Playing;
        game.currentSaveName.clear();

        InitializeEmptyBoard(game);
    }

    void ResetCurrentMatch(GameSession& game) {
        InitializeEmptyBoard(game);
        game.screen = ScreenState::Playing;
        game.currentSaveName.clear();
    }

    int GetBoardSize(const GameSession& game) {
        return (int)game.board.size();
    }

    bool IsInsideBoard(const GameSession& game, Position pos) {
        int n = (int)game.board.size();
        return pos.row >= 0 && pos.row < n && pos.col >= 0 && pos.col < n;
    }

    bool IsCellEmpty(const GameSession& game, Position pos) {
        if (!IsInsideBoard(game, pos)) return false;
        return game.board[pos.row][pos.col] == CellState::Empty;
    }

    CellState GetCell(const GameSession& game, Position pos) {
        if (!IsInsideBoard(game, pos)) return CellState::Empty;
        return game.board[pos.row][pos.col];
    }

    const std::vector<std::vector<CellState> >& GetBoard(const GameSession& game) {
        return game.board;
    }

    ActionResult PlaceCurrentTurn(GameSession& game, Position pos) {
        if (game.screen != ScreenState::Playing || game.isPaused || game.result != GameResult::InProgress) {
            return ActionResult::InvalidState;
        }

        if (!IsInsideBoard(game, pos)) {
            return ActionResult::OutOfBounds;
        }

        if (!IsCellEmpty(game, pos)) {
            return ActionResult::Occupied;
        }

        game.board[pos.row][pos.col] = game.currentTurn;
        ++game.moveCount;

        if (game.currentTurn == CellState::X) ++game.xMoveCount;
        if (game.currentTurn == CellState::O) ++game.oMoveCount;

        game.history.push_back(Move(pos, game.currentTurn, game.moveCount));

        game.result = EvaluateBoard(game, pos);

        if (game.result == GameResult::XWin) {
            ++game.xWinCount;
        }
        else if (game.result == GameResult::OWin) {
            ++game.oWinCount;
        }
        else if (game.result == GameResult::Draw) {
            ++game.drawCount;
        }
        else {
            SwitchTurn(game);
        }

        return ActionResult::Success;
    }

    void SwitchTurn(GameSession& game) {
        game.currentTurn = OpponentOf(game.currentTurn);
    }

    bool IsBoardFull(const GameSession& game) {
        int n = (int)game.board.size();
        return game.moveCount >= n * n;
    }

    std::vector<Position> GetAvailableMoves(const GameSession& game) {
        std::vector<Position> moves;

        int n = (int)game.board.size();
        for (int r = 0; r < n; ++r) {
            for (int c = 0; c < n; ++c) {
                if (game.board[r][c] == CellState::Empty) {
                    moves.push_back(Position(r, c));
                }
            }
        }

        return moves;
    }

} // namespace caro