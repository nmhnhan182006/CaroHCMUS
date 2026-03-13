#pragma once

#include "Config.h"

#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <limits>
#include <sstream>

namespace caro {

    // =====================================================
    // Enum cơ bản
    // =====================================================

    // 0 = ô trống, -1 = X, 1 = O
    enum class CellState : std::int8_t {
        Empty = 0,
        X = -1,
        O = 1
    };

    // -1 = X thắng, 0 = hòa, 1 = O thắng, 2 = chưa kết thúc
    enum class GameResult : std::int8_t {
        XWin = -1,
        Draw = 0,
        OWin = 1,
        InProgress = 2
    };

    enum class GameMode : std::uint8_t {
        PVP = 0,
        PVE = 1
    };

    enum class RuleMode : std::uint8_t {
        FreeStyle = 0,
        Standard = 1
    };

    enum class AIDifficulty : std::uint8_t {
        Easy = 0,
        Medium = 1,
        Hard = 2,
        Master = 3
    };

    enum class Language : std::uint8_t {
        Vietnamese = 0,
        English = 1
    };

    enum class ScreenState : std::uint8_t {
        MainMenu = 0,
        Settings = 1,
        LoadMenu = 2,
        Playing = 3,
        Paused = 4,
        Result = 5,
        Exit = 6
    };

    enum class ActionResult : std::uint8_t {
        Success = 0,
        OutOfBounds = 1,
        Occupied = 2,
        InvalidState = 3
    };

    // =====================================================
    // Struct dữ liệu cơ bản
    // =====================================================

    struct Position {
        int row;
        int col;

        Position() : row(-1), col(-1) {}
        Position(int r, int c) : row(r), col(c) {}
    };

    struct Move {
        Position pos;
        CellState symbol;
        int moveNumber;

        Move() : pos(), symbol(CellState::Empty), moveNumber(0) {}
        Move(Position p, CellState s, int num) : pos(p), symbol(s), moveNumber(num) {}
    };

    struct PlayerProfile {
        std::string name;
        CellState symbol;
        bool isBot;
        AIDifficulty difficulty;

        PlayerProfile()
            : name(""),
            symbol(CellState::Empty),
            isBot(false),
            difficulty(AIDifficulty::Easy) {
        }
    };

    struct GameSettings {
        int boardSize;
        GameMode gameMode;
        RuleMode ruleMode;
        AIDifficulty aiDifficulty;

        bool soundEnabled;
        bool musicEnabled;
        int soundVolume;
        int musicVolume;

        Language language;

        GameSettings()
            : boardSize(config::DEFAULT_BOARD_SIZE),
            gameMode(GameMode::PVP),
            ruleMode(RuleMode::FreeStyle),
            aiDifficulty(AIDifficulty::Easy),
            soundEnabled(config::DEFAULT_SOUND_ENABLED),
            musicEnabled(config::DEFAULT_MUSIC_ENABLED),
            soundVolume(config::DEFAULT_SOUND_VOLUME),
            musicVolume(config::DEFAULT_MUSIC_VOLUME),
            language(Language::Vietnamese) {
        }
    };

    struct SaveMetadata {
        std::string saveName;
        std::string displayDate;
        std::string displayTime;
        GameMode gameMode;
        RuleMode ruleMode;

        SaveMetadata()
            : saveName(""),
            displayDate(""),
            displayTime(""),
            gameMode(GameMode::PVP),
            ruleMode(RuleMode::FreeStyle) {
        }
    };

    struct GameSession {
        GameSettings settings;

        PlayerProfile playerX;
        PlayerProfile playerO;

        std::vector<std::vector<CellState> > board;
        std::vector<Move> history;

        CellState currentTurn;
        GameResult result;
        ScreenState screen;

        bool isPaused;
        int moveCount;

        int xMoveCount;
        int oMoveCount;

        int xWinCount;
        int oWinCount;
        int drawCount;

        std::string currentSaveName;

        GameSession()
            : currentTurn(CellState::X),
            result(GameResult::InProgress),
            screen(ScreenState::MainMenu),
            isPaused(false),
            moveCount(0),
            xMoveCount(0),
            oMoveCount(0),
            xWinCount(0),
            oWinCount(0),
            drawCount(0),
            currentSaveName("") {
            playerX.name = config::DEFAULT_PLAYER_X_NAME;
            playerX.symbol = CellState::X;
            playerX.isBot = false;
            playerX.difficulty = AIDifficulty::Easy;

            playerO.name = config::DEFAULT_PLAYER_O_NAME;
            playerO.symbol = CellState::O;
            playerO.isBot = false;
            playerO.difficulty = AIDifficulty::Easy;
        }
    };

    // =====================================================
    // Hàm tiện ích inline nhỏ
    // =====================================================

    inline bool IsPlayerCell(CellState value) {
        return value == CellState::X || value == CellState::O;
    }

    inline CellState OpponentOf(CellState value) {
        if (value == CellState::X) return CellState::O;
        if (value == CellState::O) return CellState::X;
        return CellState::Empty;
    }

    inline bool IsValidPosition(const Position& pos) {
        return pos.row >= 0 && pos.col >= 0;
    }

} // namespace caro