#include "GameAPI.h"

namespace caro {

    const char* ToString(CellState value) {
        switch (value) {
        case CellState::Empty: return "Empty";
        case CellState::X:     return "X";
        case CellState::O:     return "O";
        default:               return "Unknown";
        }
    }

    const char* ToString(GameResult value) {
        switch (value) {
        case GameResult::InProgress: return "InProgress";
        case GameResult::XWin:       return "XWin";
        case GameResult::OWin:       return "OWin";
        case GameResult::Draw:       return "Draw";
        default:                     return "Unknown";
        }
    }

    const char* ToString(GameMode value) {
        switch (value) {
        case GameMode::PVP: return "PVP";
        case GameMode::PVE: return "PVE";
        default:            return "Unknown";
        }
    }

    const char* ToString(RuleMode value) {
        switch (value) {
        case RuleMode::FreeStyle: return "FreeStyle";
        case RuleMode::Standard:  return "Standard";
        default:                  return "Unknown";
        }
    }

    const char* ToString(AIDifficulty value) {
        switch (value) {
        case AIDifficulty::Easy:   return "Easy";
        case AIDifficulty::Medium: return "Medium";
        case AIDifficulty::Hard:   return "Hard";
        case AIDifficulty::Master: return "Master";
        default:                   return "Unknown";
        }
    }

    const char* ToString(Language value) {
        switch (value) {
        case Language::Vietnamese: return "Vietnamese";
        case Language::English:    return "English";
        default:                   return "Unknown";
        }
    }

} // namespace caro