#include "SaveLoad.h"

#include <cstdio>
#include <ctime>
#include <fstream>
#include <limits>
#include <sstream>
#include <vector>

namespace caro {

    namespace {

        int ClampBoardSize(int value) {
            if (value < config::MIN_BOARD_SIZE) return config::MIN_BOARD_SIZE;
            if (value > config::MAX_BOARD_SIZE) return config::MAX_BOARD_SIZE;
            return value;
        }

        bool FileExists(const std::string& path) {
            std::ifstream fin(path.c_str());
            return fin.good();
        }

        std::string GetCurrentDateString() {
            std::time_t now = std::time(NULL);
            std::tm localTm;
#if defined(_MSC_VER)
            localtime_s(&localTm, &now);
#else
            localTm = *std::localtime(&now);
#endif

            char buffer[32];
            std::strftime(buffer, sizeof(buffer), "%d/%m/%Y", &localTm);
            return std::string(buffer);
        }

        std::string GetCurrentTimeString() {
            std::time_t now = std::time(NULL);
            std::tm localTm;
#if defined(_MSC_VER)
            localtime_s(&localTm, &now);
#else
            localTm = *std::localtime(&now);
#endif

            char buffer[32];
            std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &localTm);
            return std::string(buffer);
        }

        bool ReadMetadataFromSaveFile(const std::string& filePath, SaveMetadata& meta) {
            std::ifstream fin(filePath.c_str());
            if (!fin.is_open()) return false;

            std::string header;
            std::getline(fin, header);
            if (header != "CARO_SAVE_V1") return false;

            int boardSize = 0;
            fin >> boardSize;

            int gameMode = 0, ruleMode = 0, aiDifficulty = 0;
            fin >> gameMode >> ruleMode >> aiDifficulty;

            int soundEnabled = 0, musicEnabled = 0, soundVolume = 0, musicVolume = 0, language = 0;
            fin >> soundEnabled >> musicEnabled >> soundVolume >> musicVolume >> language;

            fin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            std::string playerXName;
            std::string playerOName;
            std::getline(fin, playerXName);
            std::getline(fin, playerOName);

            int currentTurn = 0, result = 0, screen = 0, isPaused = 0;
            fin >> currentTurn >> result >> screen >> isPaused;

            int moveCount = 0, xMoveCount = 0, oMoveCount = 0, xWinCount = 0, oWinCount = 0, drawCount = 0;
            fin >> moveCount >> xMoveCount >> oMoveCount >> xWinCount >> oWinCount >> drawCount;

            fin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            std::getline(fin, meta.saveName);
            std::getline(fin, meta.displayDate);
            std::getline(fin, meta.displayTime);

            meta.gameMode = (GameMode)gameMode;
            meta.ruleMode = (RuleMode)ruleMode;

            return true;
        }

    } // namespace

    bool SaveGameToFile(const GameSession& game, const std::string& filePath) {
        std::ofstream fout(filePath.c_str());
        if (!fout.is_open()) return false;

        fout << "CARO_SAVE_V1\n";
        fout << game.settings.boardSize << "\n";
        fout << (int)game.settings.gameMode << " "
            << (int)game.settings.ruleMode << " "
            << (int)game.settings.aiDifficulty << "\n";

        fout << (game.settings.soundEnabled ? 1 : 0) << " "
            << (game.settings.musicEnabled ? 1 : 0) << " "
            << game.settings.soundVolume << " "
            << game.settings.musicVolume << " "
            << (int)game.settings.language << "\n";

        fout << game.playerX.name << "\n";
        fout << game.playerO.name << "\n";

        fout << (int)game.currentTurn << " "
            << (int)game.result << " "
            << (int)game.screen << " "
            << (game.isPaused ? 1 : 0) << "\n";

        fout << game.moveCount << " "
            << game.xMoveCount << " "
            << game.oMoveCount << " "
            << game.xWinCount << " "
            << game.oWinCount << " "
            << game.drawCount << "\n";

        fout << game.currentSaveName << "\n";
        fout << GetCurrentDateString() << "\n";
        fout << GetCurrentTimeString() << "\n";

        int n = (int)game.board.size();
        for (int r = 0; r < n; ++r) {
            for (int c = 0; c < n; ++c) {
                fout << (int)game.board[r][c];
                if (c + 1 < n) fout << " ";
            }
            fout << "\n";
        }

        fout << game.history.size() << "\n";
        for (size_t i = 0; i < game.history.size(); ++i) {
            fout << game.history[i].pos.row << " "
                << game.history[i].pos.col << " "
                << (int)game.history[i].symbol << " "
                << game.history[i].moveNumber << "\n";
        }

        return true;
    }

    bool LoadGameFromFile(GameSession& game, const std::string& filePath) {
        std::ifstream fin(filePath.c_str());
        if (!fin.is_open()) return false;

        std::string header;
        std::getline(fin, header);
        if (header != "CARO_SAVE_V1") return false;

        int boardSize = 0;
        fin >> boardSize;
        boardSize = ClampBoardSize(boardSize);

        int gameMode = 0, ruleMode = 0, aiDifficulty = 0;
        fin >> gameMode >> ruleMode >> aiDifficulty;

        int soundEnabled = 0, musicEnabled = 0, soundVolume = 0, musicVolume = 0, language = 0;
        fin >> soundEnabled >> musicEnabled >> soundVolume >> musicVolume >> language;

        fin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::string playerXName;
        std::string playerOName;
        std::getline(fin, playerXName);
        std::getline(fin, playerOName);

        int currentTurn = 0, result = 0, screen = 0, isPaused = 0;
        fin >> currentTurn >> result >> screen >> isPaused;

        int moveCount = 0, xMoveCount = 0, oMoveCount = 0, xWinCount = 0, oWinCount = 0, drawCount = 0;
        fin >> moveCount >> xMoveCount >> oMoveCount >> xWinCount >> oWinCount >> drawCount;

        fin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::string currentSaveName;
        std::string saveDate;
        std::string saveTime;

        std::getline(fin, currentSaveName);
        std::getline(fin, saveDate);
        std::getline(fin, saveTime);

        game.settings.boardSize = boardSize;
        game.settings.gameMode = (GameMode)gameMode;
        game.settings.ruleMode = (RuleMode)ruleMode;
        game.settings.aiDifficulty = (AIDifficulty)aiDifficulty;
        game.settings.soundEnabled = (soundEnabled != 0);
        game.settings.musicEnabled = (musicEnabled != 0);
        game.settings.soundVolume = soundVolume;
        game.settings.musicVolume = musicVolume;
        game.settings.language = (Language)language;

        game.playerX.name = playerXName;
        game.playerX.symbol = CellState::X;
        game.playerX.isBot = false;
        game.playerX.difficulty = AIDifficulty::Easy;

        game.playerO.name = playerOName;
        game.playerO.symbol = CellState::O;
        game.playerO.isBot = (game.settings.gameMode == GameMode::PVE);
        game.playerO.difficulty = game.settings.aiDifficulty;

        game.currentTurn = (CellState)currentTurn;
        game.result = (GameResult)result;
        game.screen = (ScreenState)screen;
        game.isPaused = (isPaused != 0);

        game.moveCount = moveCount;
        game.xMoveCount = xMoveCount;
        game.oMoveCount = oMoveCount;
        game.xWinCount = xWinCount;
        game.oWinCount = oWinCount;
        game.drawCount = drawCount;
        game.currentSaveName = currentSaveName;

        game.board.assign(
            boardSize,
            std::vector<CellState>(boardSize, CellState::Empty)
        );

        for (int r = 0; r < boardSize; ++r) {
            for (int c = 0; c < boardSize; ++c) {
                int value = 0;
                fin >> value;
                game.board[r][c] = (CellState)value;
            }
        }

        size_t historySize = 0;
        fin >> historySize;

        game.history.clear();
        for (size_t i = 0; i < historySize; ++i) {
            int row = 0, col = 0, symbol = 0, moveNumber = 0;
            fin >> row >> col >> symbol >> moveNumber;
            game.history.push_back(
                Move(Position(row, col), (CellState)symbol, moveNumber)
            );
        }

        return true;
    }

    bool DeleteSaveFile(const std::string& filePath) {
        return std::remove(filePath.c_str()) == 0;
    }

    bool RenameSaveFile(const std::string& oldPath, const std::string& newPath) {
        return std::rename(oldPath.c_str(), newPath.c_str()) == 0;
    }

    std::vector<SaveMetadata> GetSaveSlots(const std::string& directoryPath) {
        std::vector<SaveMetadata> result;

        for (int i = 1; i <= config::MAX_SAVE_SLOTS; ++i) {
            std::ostringstream oss;
            oss << directoryPath << "slot" << i << config::DEFAULT_SAVE_EXTENSION;
            std::string path = oss.str();

            if (FileExists(path)) {
                SaveMetadata meta;
                if (ReadMetadataFromSaveFile(path, meta)) {
                    if (meta.saveName.empty()) {
                        std::ostringstream slotName;
                        slotName << "slot" << i;
                        meta.saveName = slotName.str();
                    }
                    result.push_back(meta);
                }
            }
        }

        return result;
    }

} // namespace caro