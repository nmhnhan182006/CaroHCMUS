#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <conio.h>
#include <direct.h>

#include "GameAPI.h"

using namespace std;
using namespace caro;

namespace {

    string NormalizeSaveDirectory() {
        string dir = config::DEFAULT_SAVE_DIRECTORY;
        while (!dir.empty() && (dir.back() == '/' || dir.back() == '\\')) {
            dir.pop_back();
        }
        if (dir.empty()) dir = "saves";
        return dir;
    }

    void EnsureSaveDirectory() {
        string dir = NormalizeSaveDirectory();
        _mkdir(dir.c_str());
    }

    string BuildSavePath(int slot) {
        ostringstream oss;
        oss << NormalizeSaveDirectory()
            << "/slot"
            << slot
            << config::DEFAULT_SAVE_EXTENSION;
        return oss.str();
    }

    bool FileExists(const string& path) {
        ifstream fin(path.c_str());
        return fin.good();
    }

    int ReadKeyUpper() {
        int ch = _getch();
        if (ch == 13) return 13;
        if (ch >= 'a' && ch <= 'z') ch = ch - 'a' + 'A';
        return ch;
    }

    void ClearScreen() {
        system("cls");
    }

    void WaitAnyKey() {
        cout << "\nNhan phim bat ky de tiep tuc...";
        _getch();
    }

    char CellToChar(CellState cell) {
        if (cell == CellState::X) return 'X';
        if (cell == CellState::O) return 'O';
        return '.';
    }

    string ResultToText(GameResult result) {
        switch (result) {
        case GameResult::XWin: return "X thang";
        case GameResult::OWin: return "O thang";
        case GameResult::Draw: return "Hoa";
        default: return "Dang choi";
        }
    }

    bool IsBotTurn(const GameSession& game) {
        return game.settings.gameMode == GameMode::PVE
            && game.currentTurn == CellState::O
            && game.result == GameResult::InProgress;
    }

    Position FindFirstEmptyCell(const GameSession& game) {
        int n = GetBoardSize(game);
        for (int r = 0; r < n; ++r) {
            for (int c = 0; c < n; ++c) {
                Position p(r, c);
                if (IsCellEmpty(game, p)) return p;
            }
        }
        return Position(0, 0);
    }

    Position FindDefaultCursor(const GameSession& game) {
        int n = GetBoardSize(game);
        Position center(n / 2, n / 2);
        if (IsInsideBoard(game, center) && IsCellEmpty(game, center)) {
            return center;
        }
        return FindFirstEmptyCell(game);
    }

    void PrintSelectableLine(const string& text, bool selected) {
        cout << (selected ? " > " : "   ") << text << "\n";
    }

    void PrintBoardWithCursor(const GameSession& game, const Position& cursor) {
        int n = GetBoardSize(game);
        cout << "     ";
        for (int c = 0; c < n; ++c) {
            cout << setw(3) << c;
        }
        cout << "\n";
        for (int r = 0; r < n; ++r) {
            cout << setw(3) << r << "  ";
            for (int c = 0; c < n; ++c) {
                char ch = CellToChar(GetCell(game, Position(r, c)));
                if (r == cursor.row && c == cursor.col) {
                    cout << "[" << ch << "]";
                }
                else {
                    cout << " " << ch << " ";
                }
            }
            cout << "\n";
        }
    }

    void PrintGameHeader(const GameSession& game, const string& message) {
        cout << "=========== CARO CONSOLE ===========\n\n";
        cout << "Mode      : " << ToString(game.settings.gameMode) << "\n";
        cout << "Rule      : " << ToString(game.settings.ruleMode) << "\n";
        cout << "Player X  : " << game.playerX.name << "\n";
        cout << "Player O  : " << game.playerO.name << "\n";
        cout << "Turn      : " << ToString(game.currentTurn) << "\n";
        cout << "Result    : " << ResultToText(game.result) << "\n";
        cout << "Moves     : " << game.moveCount << "\n";
        if (!game.currentSaveName.empty()) {
            cout << "Save name : " << game.currentSaveName << "\n";
        }
        if (!message.empty()) {
            cout << "Message   : " << message << "\n";
        }
        cout << "\n";
    }

    void PrintControls() {
        cout << "\n===== DIEU KHIEN =====\n";
        cout << "W/A/S/D : Di chuyen\n";
        cout << "Enter   : Chon / Danh co\n";
        cout << "P / ESC : Menu tam dung\n";
        cout << "Trong menu: W/S de chon, A/D de doi option, Enter de xac nhan\n";
    }

    void DrawGameScreen(const GameSession& game, const Position& cursor, const string& message) {
        ClearScreen();
        PrintGameHeader(game, message);
        PrintBoardWithCursor(game, cursor);
        PrintControls();
    }

    string BuildAutoSaveName(const GameSession& game, int slot) {
        ostringstream oss;
        oss << "slot" << slot
            << "_"
            << ToString(game.settings.gameMode)
            << "_"
            << game.moveCount
            << "moves";
        return oss.str();
    }

    string GetSlotDisplayText(int slot) {
        string path = BuildSavePath(slot);
        if (!FileExists(path)) {
            ostringstream oss;
            oss << "Slot " << slot << " : [Trong]";
            return oss.str();
        }
        GameSession temp;
        if (!LoadGameFromFile(temp, path)) {
            ostringstream oss;
            oss << "Slot " << slot << " : [File loi]";
            return oss.str();
        }
        ostringstream oss;
        oss << "Slot " << slot << " : ";
        if (!temp.currentSaveName.empty()) oss << temp.currentSaveName;
        else oss << "Unnamed";
        oss << " | " << temp.playerX.name << " vs " << temp.playerO.name;
        oss << " | " << temp.moveCount << " moves";
        return oss.str();
    }

    void RunBotMove(GameSession& game, string& message) {
        if (!IsBotTurn(game)) return;
        Position aiMove = FindBestAIMove(game);
        if (!IsValidPosition(aiMove)) return;
        ActionResult result = PlaceCurrentTurn(game, aiMove);
        if (result == ActionResult::Success) {
            ostringstream oss;
            oss << "BOT danh vao (" << aiMove.row << ", " << aiMove.col << ")";
            message = oss.str();
        }
    }

    bool ShowSaveMenu(GameSession& game) {
        int selected = 0;
        string message;
        while (true) {
            ClearScreen();
            cout << "=========== SAVE GAME ===========\n\n";
            for (int i = 0; i < config::MAX_SAVE_SLOTS; ++i) {
                PrintSelectableLine(GetSlotDisplayText(i + 1), selected == i);
            }
            PrintSelectableLine("Quay lai", selected == config::MAX_SAVE_SLOTS);
            cout << "\nW/S de chon, Enter de save.\n";
            if (!message.empty()) cout << "\n" << message << "\n";
            int key = ReadKeyUpper();
            if (key == 'W') {
                selected--;
                if (selected < 0) selected = config::MAX_SAVE_SLOTS;
            }
            else if (key == 'S') {
                selected++;
                if (selected > config::MAX_SAVE_SLOTS) selected = 0;
            }
            else if (key == 13) {
                if (selected == config::MAX_SAVE_SLOTS) return false;
                int slot = selected + 1;
                string path = BuildSavePath(slot);
                game.currentSaveName = BuildAutoSaveName(game, slot);
                if (SaveGameToFile(game, path)) {
                    message = "Save thanh cong vao slot " + to_string(slot);
                    ClearScreen();
                    cout << "=========== SAVE GAME ===========\n\n";
                    cout << message << "\n";
                    WaitAnyKey();
                    return true;
                }
                else {
                    message = "Save that bai";
                }
            }
            else if (key == 27) {
                return false;
            }
        }
    }

    void ChangeMode(GameSettings& settings) {
        settings.gameMode = (settings.gameMode == GameMode::PVP) ? GameMode::PVE : GameMode::PVP;
    }

    void ChangeRule(GameSettings& settings) {
        settings.ruleMode = (settings.ruleMode == RuleMode::FreeStyle) ? RuleMode::Standard : RuleMode::FreeStyle;
    }

    void ChangeAIDifficulty(GameSettings& settings, int direction) {
        int value = (int)settings.aiDifficulty;
        value += direction;
        if (value < 0) value = 3;
        if (value > 3) value = 0;
        settings.aiDifficulty = (AIDifficulty)value;
    }

    bool ShowNewGameMenu(GameSession& game) {
        GameSettings settings = CreateDefaultSettings();
        int selected = 0;
        while (true) {
            ClearScreen();
            cout << "=========== NEW GAME ===========\n\n";
            PrintSelectableLine("Game Mode   : " + string(ToString(settings.gameMode)), selected == 0);
            PrintSelectableLine("Rule Mode   : " + string(ToString(settings.ruleMode)), selected == 1);
            PrintSelectableLine("AI Level    : " + string(ToString(settings.aiDifficulty)), selected == 2);
            PrintSelectableLine("Bat dau game", selected == 3);
            PrintSelectableLine("Quay lai", selected == 4);
            cout << "\nW/S de di chuyen, A/D de doi option, Enter de chon.\n";
            int key = ReadKeyUpper();
            if (key == 'W') {
                selected--;
                if (selected < 0) selected = 4;
            }
            else if (key == 'S') {
                selected++;
                if (selected > 4) selected = 0;
            }
            else if (key == 'A') {
                if (selected == 0) ChangeMode(settings);
                else if (selected == 1) ChangeRule(settings);
                else if (selected == 2) ChangeAIDifficulty(settings, -1);
            }
            else if (key == 'D') {
                if (selected == 0) ChangeMode(settings);
                else if (selected == 1) ChangeRule(settings);
                else if (selected == 2) ChangeAIDifficulty(settings, 1);
            }
            else if (key == 13) {
                if (selected == 3) {
                    string playerX = config::DEFAULT_PLAYER_X_NAME;
                    string playerO = (settings.gameMode == GameMode::PVE)
                        ? string(config::DEFAULT_BOT_NAME)
                        : string(config::DEFAULT_PLAYER_O_NAME);
                    StartNewGame(game, settings, playerX, playerO);
                    game.screen = ScreenState::Playing;
                    return true;
                }
                else if (selected == 4) {
                    return false;
                }
            }
            else if (key == 27) {
                return false;
            }
        }
    }

    enum class PauseAction {
        ContinueGame,
        RestartGame,
        BackToMainMenu
    };

    PauseAction ShowPauseMenu(GameSession& game) {
        int selected = 0;
        while (true) {
            ClearScreen();
            cout << "=========== MENU TAM DUNG ===========\n\n";
            PrintSelectableLine("Tiep tuc", selected == 0);
            PrintSelectableLine("Save game", selected == 1);
            PrintSelectableLine("Choi lai tran nay", selected == 2);
            PrintSelectableLine("Ve menu chinh", selected == 3);
            cout << "\nW/S de chon, Enter de xac nhan.\n";
            int key = ReadKeyUpper();
            if (key == 'W') {
                selected--;
                if (selected < 0) selected = 3;
            }
            else if (key == 'S') {
                selected++;
                if (selected > 3) selected = 0;
            }
            else if (key == 13) {
                if (selected == 0) return PauseAction::ContinueGame;
                if (selected == 1) ShowSaveMenu(game);
                else if (selected == 2) return PauseAction::RestartGame;
                else if (selected == 3) return PauseAction::BackToMainMenu;
            }
            else if (key == 27) {
                return PauseAction::ContinueGame;
            }
        }
    }

    bool ShowResultMenu(GameSession& game) {
        int selected = 0;
        while (true) {
            ClearScreen();
            cout << "=========== KET THUC VAN ===========\n\n";
            cout << "Ket qua: " << ResultToText(game.result) << "\n\n";
            PrintSelectableLine("Choi lai", selected == 0);
            PrintSelectableLine("Save game", selected == 1);
            PrintSelectableLine("Ve menu chinh", selected == 2);
            cout << "\nW/S de chon, Enter de xac nhan.\n";
            int key = ReadKeyUpper();
            if (key == 'W') {
                selected--;
                if (selected < 0) selected = 2;
            }
            else if (key == 'S') {
                selected++;
                if (selected > 2) selected = 0;
            }
            else if (key == 13) {
                if (selected == 0) {
                    ResetCurrentMatch(game);
                    return true;
                }
                else if (selected == 1) ShowSaveMenu(game);
                else if (selected == 2) return false;
            }
        }
    }

    int ShowMainMenu() {
        int selected = 0;
        while (true) {
            ClearScreen();
            cout << "=========== CARO GAME ===========\n\n";
            PrintSelectableLine("New Game", selected == 0);
            PrintSelectableLine("Load Game", selected == 1);
            PrintSelectableLine("Exit", selected == 2);
            cout << "\nW/S de di chuyen, Enter de chon.\n";
            int key = ReadKeyUpper();
            if (key == 'W') {
                selected--;
                if (selected < 0) selected = 2;
            }
            else if (key == 'S') {
                selected++;
                if (selected > 2) selected = 0;
            }
            else if (key == 13) {
                return selected;
            }
        }
    }

} // namespace

// ===== CÁC HÀM PUBLIC — nằm ngoài namespace =====

void RunGameLoop(caro::GameSession& game) {
    Position cursor = FindDefaultCursor(game);
    string message;
    while (true) {
        if (IsBotTurn(game)) {
            RunBotMove(game, message);
            cursor = FindDefaultCursor(game);
        }
        if (game.result != GameResult::InProgress) {
            bool continuePlaying = ShowResultMenu(game);
            if (!continuePlaying) return;
            cursor = FindDefaultCursor(game);
            message.clear();
            continue;
        }
        DrawGameScreen(game, cursor, message);
        int key = ReadKeyUpper();
        if (key == 'W') {
            if (cursor.row > 0) cursor.row--;
        }
        else if (key == 'S') {
            if (cursor.row + 1 < GetBoardSize(game)) cursor.row++;
        }
        else if (key == 'A') {
            if (cursor.col > 0) cursor.col--;
        }
        else if (key == 'D') {
            if (cursor.col + 1 < GetBoardSize(game)) cursor.col++;
        }
        else if (key == 13) {
            ActionResult result = PlaceCurrentTurn(game, cursor);
            if (result == ActionResult::Success) message = "Danh co thanh cong";
            else if (result == ActionResult::Occupied) message = "O nay da duoc danh";
            else if (result == ActionResult::OutOfBounds) message = "Vi tri ngoai ban co";
            else message = "Khong the danh nuoc nay";
        }
        else if (key == 'P' || key == 27) {
            PauseAction action = ShowPauseMenu(game);
            if (action == PauseAction::ContinueGame) message = "Tiep tuc game";
            else if (action == PauseAction::RestartGame) {
                ResetCurrentMatch(game);
                cursor = FindDefaultCursor(game);
                message = "Da bat dau lai tran dau";
            }
            else if (action == PauseAction::BackToMainMenu) return;
        }
    }
}

bool ShowLoadMenu(caro::GameSession& game) {
    int selected = 0;
    string message;
    while (true) {
        ClearScreen();
        cout << "=========== LOAD GAME ===========\n\n";
        for (int i = 0; i < config::MAX_SAVE_SLOTS; ++i) {
            PrintSelectableLine(GetSlotDisplayText(i + 1), selected == i);
        }
        PrintSelectableLine("Quay lai", selected == config::MAX_SAVE_SLOTS);
        cout << "\nW/S de chon, Enter de load.\n";
        if (!message.empty()) cout << "\n" << message << "\n";
        int key = ReadKeyUpper();
        if (key == 'W') {
            selected--;
            if (selected < 0) selected = config::MAX_SAVE_SLOTS;
        }
        else if (key == 'S') {
            selected++;
            if (selected > config::MAX_SAVE_SLOTS) selected = 0;
        }
        else if (key == 13) {
            if (selected == config::MAX_SAVE_SLOTS) return false;
            int slot = selected + 1;
            string path = BuildSavePath(slot);
            if (!FileExists(path)) {
                message = "Slot nay dang trong";
                continue;
            }
            if (LoadGameFromFile(game, path)) {
                game.screen = ScreenState::Playing;
                game.isPaused = false;
                return true;
            }
            else {
                message = "Load that bai";
            }
        }
        else if (key == 27) {
            return false;
        }
    }
}

void RunConsoleGame() {
    EnsureSaveDirectory();
    while (true) {
        int choice = ShowMainMenu();
        if (choice == 0) {
            GameSession game;
            if (ShowNewGameMenu(game)) RunGameLoop(game);
        }
        else if (choice == 1) {
            GameSession game;
            if (ShowLoadMenu(game)) RunGameLoop(game);
        }
        else if (choice == 2) { break; }
    }
}

void RunNewGame() {
    EnsureSaveDirectory();
    GameSession game;
    if (ShowNewGameMenu(game)) RunGameLoop(game);
}