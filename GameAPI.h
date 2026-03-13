#pragma once

// =====================================================
// Header tong cho Caro Engine
// main.cpp chi can include file nay
// =====================================================

#include "GameSetting.h"
#include "GameSession.h"
#include "GameCore.h"
#include "GameRules.h"
#include "GameAI.h"
#include "SaveLoad.h"

namespace caro {

    const char* ToString(CellState value);
    const char* ToString(GameResult value);
    const char* ToString(GameMode value);
    const char* ToString(RuleMode value);
    const char* ToString(AIDifficulty value);
    const char* ToString(Language value);

} // namespace caro