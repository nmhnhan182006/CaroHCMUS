#pragma once

namespace caro {
	namespace config {

		// =========================
		// Board / Rule
		// =========================
		static const int DEFAULT_BOARD_SIZE = 15;
		static const int MIN_BOARD_SIZE = 10;
		static const int MAX_BOARD_SIZE = 30;
		static const int WIN_LENGTH = 5;

		// =========================
		// Save / Load
		// =========================
		static const int MAX_SAVE_SLOTS = 5;
		static const char* const DEFAULT_SAVE_DIRECTORY = "saves/";
		static const char* const DEFAULT_SAVE_EXTENSION = ".caro";

		// =========================
		// Audio / UI defaults
		// =========================
		static const bool DEFAULT_SOUND_ENABLED = true;
		static const bool DEFAULT_MUSIC_ENABLED = true;
		static const int DEFAULT_SOUND_VOLUME = 100;
		static const int DEFAULT_MUSIC_VOLUME = 100;

		// =========================
		// Text defaults
		// =========================
		static const char* const DEFAULT_LANGUAGE = "vi";
		static const char* const DEFAULT_PLAYER_X_NAME = "Player 1";
		static const char* const DEFAULT_PLAYER_O_NAME = "Player 2";
		static const char* const DEFAULT_BOT_NAME = "BOT";

		// =========================
// Input defaults
// =========================
		static const char KEY_UP = 'W';
		static const char KEY_DOWN = 'S';
		static const char KEY_LEFT = 'A';
		static const char KEY_RIGHT = 'D';
		static const char KEY_CONFIRM = 13;   // Enter
		static const char KEY_SAVE = 'L';
		static const char KEY_LOAD = 'T';
		static const char KEY_MENU = 'P';     // P mo menu tam dung
		static const char KEY_PAUSE = 27;     // ESC

	} // namespace config
} // namespace caro