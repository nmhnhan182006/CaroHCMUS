#pragma once

#include "Types.h"

namespace caro {

    // Tạo settings mặc định
    GameSettings CreateDefaultSettings();

    // In thông tin settings
    void PrintSettings(const GameSettings& settings);

}