#pragma once

#include "Types.h"

namespace caro {

    // =====================================================
    // Save / Load
    // =====================================================

    bool SaveGameToFile(const GameSession& game, const std::string& filePath);

    bool LoadGameFromFile(GameSession& game, const std::string& filePath);

    bool DeleteSaveFile(const std::string& filePath);

    bool RenameSaveFile(const std::string& oldPath, const std::string& newPath);

    std::vector<SaveMetadata> GetSaveSlots(
        const std::string& directoryPath = config::DEFAULT_SAVE_DIRECTORY
    );

} // namespace caro