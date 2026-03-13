#include "GameSetting.h"
#include "GameAPI.h"
#include <iostream>

namespace caro {

    GameSettings CreateDefaultSettings() {
        GameSettings settings;

        settings.boardSize = config::DEFAULT_BOARD_SIZE;
        settings.gameMode = GameMode::PVP;
        settings.ruleMode = RuleMode::FreeStyle;
        settings.aiDifficulty = AIDifficulty::Easy;

        settings.soundEnabled = config::DEFAULT_SOUND_ENABLED;
        settings.musicEnabled = config::DEFAULT_MUSIC_ENABLED;

        settings.soundVolume = config::DEFAULT_SOUND_VOLUME;
        settings.musicVolume = config::DEFAULT_MUSIC_VOLUME;

        settings.language = Language::Vietnamese;

        return settings;
    }

    void PrintSettings(const GameSettings& settings) {
        std::cout << "Board size : " << settings.boardSize << "\n";
        std::cout << "Game mode  : " << ToString(settings.gameMode) << "\n";
        std::cout << "Rule mode  : " << ToString(settings.ruleMode) << "\n";
        std::cout << "AI level   : " << ToString(settings.aiDifficulty) << "\n";
    }

}