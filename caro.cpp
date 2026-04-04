#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>
#include "GameAPI.h"
#include "GameSession.h"

using namespace std;

// =============================================
// FORWARD DECLARATIONS
// =============================================
void RunConsoleGame();
bool ShowLoadMenu(caro::GameSession& game);
void RunGameLoop(caro::GameSession& game);
sf::ConvexShape makeScifiBox(float x, float y, float w, float h, float cut);
void initLoadScreen();
void initSettingsScreen();
void RunNewGame();

// =============================================
// ASSETS PATH
// =============================================
string getAssetsPath() {
	namespace fs = std::filesystem;
	fs::path currentPath = fs::current_path();
	vector<fs::path> possiblePaths = {
		currentPath / "assets",
		currentPath / ".." / "assets",
		currentPath / ".." / ".." / "assets",
		currentPath / ".." / ".." / ".." / "assets"
	};
	for (const auto& path : possiblePaths) {
		fs::path canonical = fs::weakly_canonical(path);
		if (fs::exists(canonical / "font") && fs::exists(canonical / "music")) {
			return canonical.string();
		}
	}
	cout << "Warning: Could not find assets folder.\n";
	return "assets";
}

// =============================================
// CONSTANTS
// =============================================
const int WIDTH = 1280;
const int HEIGHT = 720;
const int ROWS = 15;
const int COLS = 15;
const float CELL = 40.f;

// =============================================
// GLOBAL OBJECTS
// =============================================
sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Caro Game");

int currentScreen = 0;
// 0 = menu, 1 = new game, 2 = settings, 3 = load game screen

// Menu
vector<string> menuItems = { "New Game", "Settings", "Load Game", "Exit" };
int selectedMenu = 0;

// Board
int board[ROWS][COLS];
int currentPlayer = 1;

// Assets
sf::Font font;
sf::Texture menuBgTexture;
sf::Sprite menuBgSprite;
sf::Texture xTexture;
sf::Texture oTexture;

// Menu shapes
sf::Clock menuClock;
vector<sf::Text>          menuTexts;
vector<sf::ConvexShape>   menuConvex;
vector<sf::ConvexShape>   menuConvexGlow;

// Title
sf::Text titleText;
sf::Text subtitleText;

// Sound
sf::Music      bgMusic;
sf::SoundBuffer hoverBuffer;
sf::Sound       hoverSound;
sf::SoundBuffer selectBuffer;
sf::Sound       selectSound;

// Particles
struct Particle {
	sf::CircleShape shape;
	sf::Vector2f    velocity;
	float           lifetime;
	float           maxLifetime;
};
vector<Particle> particles;

// Load Game Screen
struct SaveSlotInfo {
	string displayText;
	bool   isEmpty;
};
vector<SaveSlotInfo>    saveSlots;
int                     selectedSlot = 0;
sf::Text                loadTitleText;
vector<sf::ConvexShape> slotConvex;
vector<sf::ConvexShape> slotConvexGlow;
vector<sf::Text>        slotTexts;
sf::Text                backButtonText;
sf::ConvexShape         backButtonShape;

// Settings Screen
caro::GameSettings currentSettings;
int selectedSetting = 0;
// 0 = Music Volume, 1 = Sound Volume, 2 = AI Difficulty, 3 = Back

sf::Text settingsTitleText;
struct SettingRow {
	sf::Text labelText;
	sf::Text valueText;
	sf::ConvexShape box;
	sf::ConvexShape glow;
};
vector<SettingRow> settingRows;
sf::ConvexShape settingsBackShape;
sf::Text settingsBackText;

// =============================================
// makeScifiBox — hình vát 2 góc trái
// =============================================
sf::ConvexShape makeScifiBox(float x, float y, float w, float h, float cut) {
	sf::ConvexShape shape;
	shape.setPointCount(6);
	shape.setPoint(0, sf::Vector2f(x + cut, y + h));
	shape.setPoint(1, sf::Vector2f(x, y + h - cut));
	shape.setPoint(2, sf::Vector2f(x, y + cut));
	shape.setPoint(3, sf::Vector2f(x + cut, y));
	shape.setPoint(4, sf::Vector2f(x + w, y));
	shape.setPoint(5, sf::Vector2f(x + w, y + h));
	return shape;
}

// =============================================
// PARTICLES
// =============================================
void initParticles() {
	srand(42);
	for (int i = 0; i < 80; i++) {
		Particle p;
		float radius = (rand() % 3 + 1) * 0.5f;
		p.shape.setRadius(radius);
		p.shape.setOrigin(radius, radius);
		p.shape.setPosition((float)(rand() % WIDTH), (float)(rand() % HEIGHT));

		int colorType = rand() % 3;
		if (colorType == 0)
			p.shape.setFillColor(sf::Color(255, 255, 255, rand() % 150 + 80));
		else if (colorType == 1)
			p.shape.setFillColor(sf::Color(180, 100, 255, rand() % 150 + 80));
		else
			p.shape.setFillColor(sf::Color(100, 200, 255, rand() % 150 + 80));

		p.velocity = sf::Vector2f(
			((rand() % 100) - 50) * 0.008f,
			((rand() % 100) - 50) * 0.008f
		);
		p.maxLifetime = (float)(rand() % 300 + 200);
		p.lifetime = (float)(rand() % (int)p.maxLifetime);
		particles.push_back(p);
	}
}

void updateAndDrawParticles() {
	float t = menuClock.getElapsedTime().asSeconds();
	for (auto& p : particles) {
		p.shape.move(p.velocity);
		p.lifetime += 1.f;
		if (p.lifetime >= p.maxLifetime) {
			p.lifetime = 0;
			p.shape.setPosition((float)(rand() % WIDTH), (float)(rand() % HEIGHT));
		}
		float alpha = (sin(p.lifetime * 0.05f) + 1.f) / 2.f;
		sf::Color c = p.shape.getFillColor();
		c.a = (sf::Uint8)(60 + alpha * 180);
		p.shape.setFillColor(c);

		sf::Vector2f pos = p.shape.getPosition();
		if (pos.x < 0)      p.shape.setPosition(WIDTH, pos.y);
		if (pos.x > WIDTH)  p.shape.setPosition(0, pos.y);
		if (pos.y < 0)      p.shape.setPosition(pos.x, HEIGHT);
		if (pos.y > HEIGHT) p.shape.setPosition(pos.x, 0);
		window.draw(p.shape);
	}

	// Shooting stars
	float starT = fmod(t * 0.4f, 1.f);
	sf::RectangleShape star(sf::Vector2f(60 + starT * 40, 1.5f));
	star.setPosition(-80 + starT * (WIDTH + 160), 150 + sin(t * 0.3f) * 80);
	star.setFillColor(sf::Color(200, 180, 255, (sf::Uint8)(80 * sin(starT * 3.14f))));
	star.setRotation(-5);
	window.draw(star);

	float starT2 = fmod(t * 0.3f + 0.5f, 1.f);
	sf::RectangleShape star2(sf::Vector2f(40 + starT2 * 30, 1.f));
	star2.setPosition(-60 + starT2 * (WIDTH + 120), 400 + sin(t * 0.5f) * 60);
	star2.setFillColor(sf::Color(150, 220, 255, (sf::Uint8)(60 * sin(starT2 * 3.14f))));
	star2.setRotation(-3);
	window.draw(star2);
}

// =============================================
// TITLE
// =============================================
void initTitle() {
	titleText.setFont(font);
	titleText.setString("CARO");
	titleText.setCharacterSize(72);
	titleText.setStyle(sf::Text::Bold);
	sf::FloatRect tr = titleText.getLocalBounds();
	titleText.setPosition((WIDTH - tr.width) / 2, 130);

	subtitleText.setFont(font);
	subtitleText.setString("HCMUS");
	subtitleText.setCharacterSize(28);
	sf::FloatRect sr = subtitleText.getLocalBounds();
	subtitleText.setPosition((WIDTH - sr.width) / 2, 220);
}

void drawTitle() {
	float t = menuClock.getElapsedTime().asSeconds();
	float r1 = (sin(t * 1.2f) + 1.f) / 2.f;
	float r2 = (sin(t * 1.2f + 2.1f) + 1.f) / 2.f;
	float r3 = (sin(t * 1.2f + 4.2f) + 1.f) / 2.f;
	sf::Color titleColor(
		(sf::Uint8)(180 + r1 * 75),
		(sf::Uint8)(80 + r2 * 100),
		(sf::Uint8)(200 + r3 * 55)
	);

	int offsets[8][2] = {
		{-3,0},{3,0},{0,-3},{0,3},
		{-3,-3},{3,-3},{-3,3},{3,3}
	};

	// Outline CARO
	sf::Text outline = titleText;
	outline.setFillColor(sf::Color(20, 0, 50, 220));
	for (auto& o : offsets) {
		outline.setPosition(titleText.getPosition().x + o[0], titleText.getPosition().y + o[1]);
		window.draw(outline);
	}
	titleText.setFillColor(titleColor);
	window.draw(titleText);

	// Glow CARO
	sf::Text glow = titleText;
	glow.setFillColor(sf::Color(titleColor.r, titleColor.g, titleColor.b, 60));
	for (int d = 6; d <= 12; d += 3) {
		glow.setPosition(titleText.getPosition().x, titleText.getPosition().y + d / 3);
		window.draw(glow);
	}

	// HCMUS
	float sub_pulse = (sin(t * 1.5f + 1.f) + 1.f) / 2.f;
	sf::Color subColor(
		(sf::Uint8)(160 + sub_pulse * 60),
		(sf::Uint8)(120 + sub_pulse * 80),
		255
	);
	sf::Text subOutline = subtitleText;
	subOutline.setFillColor(sf::Color(20, 0, 50, 200));
	for (auto& o : offsets) {
		subOutline.setPosition(
			subtitleText.getPosition().x + o[0] * 0.6f,
			subtitleText.getPosition().y + o[1] * 0.6f
		);
		window.draw(subOutline);
	}
	subtitleText.setFillColor(subColor);
	window.draw(subtitleText);

	// Đường trang trí
	float lineW = 200.f;
	float lineY = subtitleText.getPosition().y - 8;
	sf::RectangleShape lineLeft(sf::Vector2f(lineW, 2));
	lineLeft.setPosition((WIDTH / 2.f) - lineW - 20, lineY);
	lineLeft.setFillColor(sf::Color(titleColor.r, titleColor.g, titleColor.b, 180));
	window.draw(lineLeft);

	sf::RectangleShape lineRight(sf::Vector2f(lineW, 2));
	lineRight.setPosition((WIDTH / 2.f) + 20, lineY);
	lineRight.setFillColor(sf::Color(titleColor.r, titleColor.g, titleColor.b, 180));
	window.draw(lineRight);

	sf::ConvexShape diamond;
	diamond.setPointCount(4);
	diamond.setPoint(0, sf::Vector2f(WIDTH / 2.f, lineY - 6));
	diamond.setPoint(1, sf::Vector2f(WIDTH / 2.f + 10, lineY));
	diamond.setPoint(2, sf::Vector2f(WIDTH / 2.f, lineY + 6));
	diamond.setPoint(3, sf::Vector2f(WIDTH / 2.f - 10, lineY));
	diamond.setFillColor(titleColor);
	window.draw(diamond);
}

// =============================================
// MENU
// =============================================
void initMenu() {
	for (int i = 0; i < (int)menuItems.size(); i++) {
		float x = WIDTH / 2 - 140;
		float y = 320 + i * 80;
		float w = 280, h = 60, cut = 18;

		sf::ConvexShape glow = makeScifiBox(x - 6, y - 6, w + 12, h + 12, cut + 4);
		glow.setFillColor(sf::Color(160, 80, 255, 50));
		glow.setOutlineThickness(0);
		menuConvexGlow.push_back(glow);

		sf::ConvexShape box = makeScifiBox(x, y, w, h, cut);
		box.setFillColor(sf::Color(10, 5, 30, 190));
		box.setOutlineColor(sf::Color(160, 80, 255));
		box.setOutlineThickness(2);
		menuConvex.push_back(box);

		sf::Text text;
		text.setFont(font);
		text.setString(menuItems[i]);
		text.setCharacterSize(22);
		text.setFillColor(sf::Color::White);
		sf::FloatRect textRect = text.getLocalBounds();
		text.setPosition(
			x + cut + (w - cut - textRect.width) / 2,
			y + (h - textRect.height) / 2 - 5
		);
		menuTexts.push_back(text);
	}
}

void drawMenu() {
	float t = menuClock.getElapsedTime().asSeconds();
	float pulse = (sin(t * 3.0f) + 1.0f) / 2.0f;

	for (int i = 0; i < (int)menuTexts.size(); i++) {
		if (i == selectedMenu) {
			sf::Uint8 r = (sf::Uint8)(60 + pulse * 80);
			sf::Uint8 g = (sf::Uint8)(10 + pulse * 40);
			sf::Uint8 b = (sf::Uint8)(160 + pulse * 80);
			menuConvex[i].setFillColor(sf::Color(r, g, b, 220));
			menuConvex[i].setOutlineColor(sf::Color(255, 220, 80));
			menuConvex[i].setOutlineThickness(3);
			menuTexts[i].setFillColor(sf::Color::Yellow);

			sf::Uint8 glowA = (sf::Uint8)(40 + pulse * 80);
			menuConvexGlow[i].setFillColor(sf::Color(180, 80, 255, glowA));
			window.draw(menuConvexGlow[i]);

			sf::Text arrow;
			arrow.setFont(font);
			arrow.setString(">");
			arrow.setCharacterSize(20);
			arrow.setFillColor(sf::Color(255, 220, 80, (sf::Uint8)(150 + pulse * 105)));
			arrow.setPosition(menuConvex[i].getPoint(2).x - 30, menuConvex[i].getPoint(2).y + 12);

			window.draw(menuConvex[i]);
			sf::Text shadow = menuTexts[i];
			shadow.move(2, 2);
			shadow.setFillColor(sf::Color(0, 0, 0, 160));
			window.draw(shadow);
			window.draw(menuTexts[i]);
			window.draw(arrow);
		}
		else {
			menuConvex[i].setFillColor(sf::Color(10, 5, 30, 170));
			menuConvex[i].setOutlineColor(sf::Color(120, 60, 200, 180));
			menuConvex[i].setOutlineThickness(2);
			menuTexts[i].setFillColor(sf::Color(200, 170, 255));

			window.draw(menuConvex[i]);
			sf::Text shadow = menuTexts[i];
			shadow.move(2, 2);
			shadow.setFillColor(sf::Color(0, 0, 0, 160));
			window.draw(shadow);
			window.draw(menuTexts[i]);
		}
	}
}

void handleMenuInput(sf::Event& event) {
	if (event.type != sf::Event::KeyPressed) return;

	if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W) {
		selectedMenu--;
		if (selectedMenu < 0) selectedMenu = (int)menuItems.size() - 1;
		hoverSound.play();
	}
	else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S) {
		selectedMenu++;
		if (selectedMenu >= (int)menuItems.size()) selectedMenu = 0;
		hoverSound.play();
	}
	else if (event.key.code == sf::Keyboard::Enter) {
		selectSound.play();
		sf::sleep(sf::milliseconds(200));

		if (menuItems[selectedMenu] == "Exit") {
			window.close();
		}
		else if (menuItems[selectedMenu] == "New Game") {
			window.setVisible(false);   // ẩn cửa sổ SFML
			bgMusic.pause();            // tạm dừng nhạc
			RunNewGame();          
			bgMusic.play();             // bật lại nhạc
			window.setVisible(true);    // hiện lại cửa sổ
			currentScreen = 0;          // về menu
			selectedMenu = 0;
		}
		else if (menuItems[selectedMenu] == "Load Game") {
			selectedSlot = 0;
			initLoadScreen();
			currentScreen = 3;
		}
		else if (menuItems[selectedMenu] == "Settings") {
			selectedSetting = 0;
			initSettingsScreen();
			currentScreen = 2;
		}
	}
}

// =============================================
// LOAD GAME SCREEN
// =============================================
string getSlotDisplayText(int slot) {
	string path = string(caro::config::DEFAULT_SAVE_DIRECTORY)
		+ "slot" + to_string(slot)
		+ caro::config::DEFAULT_SAVE_EXTENSION;

	ifstream fin(path.c_str());
	if (!fin.good()) return "[ TRONG ]";

	caro::GameSession temp;
	if (!caro::LoadGameFromFile(temp, path)) return "[ LOI FILE ]";

	string result = "";
	if (!temp.currentSaveName.empty()) result += temp.currentSaveName + " | ";
	result += temp.playerX.name + " vs " + temp.playerO.name;
	result += " | " + to_string(temp.moveCount) + " nuoc";
	return result;
}

void initLoadScreen() {
	saveSlots.clear();
	slotConvex.clear();
	slotConvexGlow.clear();
	slotTexts.clear();

	for (int i = 0; i < caro::config::MAX_SAVE_SLOTS; i++) {
		SaveSlotInfo info;
		string slotText = getSlotDisplayText(i + 1);
		info.displayText = "Slot " + to_string(i + 1) + ":  " + slotText;
		info.isEmpty = (slotText == "[ TRONG ]");
		saveSlots.push_back(info);
	}

	float startY = 180.f;
	float x = WIDTH / 2 - 300;
	float w = 600, h = 60, cut = 18;

	for (int i = 0; i < caro::config::MAX_SAVE_SLOTS; i++) {
		float y = startY + i * 80;

		sf::ConvexShape glow = makeScifiBox(x - 6, y - 6, w + 12, h + 12, cut + 4);
		glow.setFillColor(sf::Color(160, 80, 255, 50));
		slotConvexGlow.push_back(glow);

		sf::ConvexShape box = makeScifiBox(x, y, w, h, cut);
		box.setFillColor(sf::Color(10, 5, 30, 190));
		box.setOutlineColor(sf::Color(160, 80, 255));
		box.setOutlineThickness(2);
		slotConvex.push_back(box);

		sf::Text text;
		text.setFont(font);
		text.setString(saveSlots[i].displayText);
		text.setCharacterSize(14);
		text.setFillColor(saveSlots[i].isEmpty ? sf::Color(100, 100, 150) : sf::Color(200, 170, 255));
		sf::FloatRect tr = text.getLocalBounds();
		text.setPosition(x + cut + 10, y + (h - tr.height) / 2 - 5);
		slotTexts.push_back(text);
	}

	// Tiêu đề
	loadTitleText.setFont(font);
	loadTitleText.setString("LOAD GAME");
	loadTitleText.setCharacterSize(36);
	sf::FloatRect tr = loadTitleText.getLocalBounds();
	loadTitleText.setPosition((WIDTH - tr.width) / 2, 80);

	// Nút Back
	backButtonShape = makeScifiBox(WIDTH / 2 - 100, 620, 200, 50, 14);
	backButtonShape.setFillColor(sf::Color(10, 5, 30, 190));
	backButtonShape.setOutlineColor(sf::Color(160, 80, 255));
	backButtonShape.setOutlineThickness(2);

	backButtonText.setFont(font);
	backButtonText.setString("< BACK");
	backButtonText.setCharacterSize(18);
	backButtonText.setFillColor(sf::Color::White);
	sf::FloatRect br = backButtonText.getLocalBounds();
	backButtonText.setPosition(WIDTH / 2 - br.width / 2, 630);
}

void drawLoadScreen() {
	float t = menuClock.getElapsedTime().asSeconds();
	float pulse = (sin(t * 3.0f) + 1.0f) / 2.0f;

	// Tiêu đề gradient
	float r1 = (sin(t * 1.2f) + 1.f) / 2.f;
	float r2 = (sin(t * 1.2f + 2.1f) + 1.f) / 2.f;
	float r3 = (sin(t * 1.2f + 4.2f) + 1.f) / 2.f;
	sf::Color titleColor(
		(sf::Uint8)(180 + r1 * 75),
		(sf::Uint8)(80 + r2 * 100),
		(sf::Uint8)(200 + r3 * 55)
	);
	loadTitleText.setFillColor(titleColor);

	int offsets[8][2] = {
		{-3,0},{3,0},{0,-3},{0,3},
		{-3,-3},{3,-3},{-3,3},{3,3}
	};
	sf::Text titleOutline = loadTitleText;
	titleOutline.setFillColor(sf::Color(20, 0, 50, 200));
	for (auto& o : offsets) {
		titleOutline.setPosition(loadTitleText.getPosition().x + o[0], loadTitleText.getPosition().y + o[1]);
		window.draw(titleOutline);
	}
	window.draw(loadTitleText);

	// Các slot
	for (int i = 0; i < (int)slotConvex.size(); i++) {
		if (i == selectedSlot) {
			sf::Uint8 r = (sf::Uint8)(60 + pulse * 80);
			sf::Uint8 g = (sf::Uint8)(10 + pulse * 40);
			sf::Uint8 b = (sf::Uint8)(160 + pulse * 80);
			slotConvex[i].setFillColor(sf::Color(r, g, b, 220));
			slotConvex[i].setOutlineColor(sf::Color(255, 220, 80));
			slotConvex[i].setOutlineThickness(3);
			slotTexts[i].setFillColor(sf::Color::Yellow);

			sf::Uint8 glowA = (sf::Uint8)(40 + pulse * 80);
			slotConvexGlow[i].setFillColor(sf::Color(180, 80, 255, glowA));
			window.draw(slotConvexGlow[i]);

			sf::Text arrow;
			arrow.setFont(font);
			arrow.setString(">");
			arrow.setCharacterSize(18);
			arrow.setFillColor(sf::Color(255, 220, 80, (sf::Uint8)(150 + pulse * 105)));
			arrow.setPosition(slotConvex[i].getPoint(2).x - 28, slotConvex[i].getPoint(2).y + 14);

			window.draw(slotConvex[i]);
			window.draw(slotTexts[i]);
			window.draw(arrow);
		}
		else {
			slotConvex[i].setFillColor(saveSlots[i].isEmpty ? sf::Color(10, 5, 30, 120) : sf::Color(10, 5, 30, 190));
			slotConvex[i].setOutlineColor(sf::Color(120, 60, 200, 180));
			slotConvex[i].setOutlineThickness(2);
			slotTexts[i].setFillColor(saveSlots[i].isEmpty ? sf::Color(80, 80, 120) : sf::Color(200, 170, 255));
			window.draw(slotConvex[i]);
			window.draw(slotTexts[i]);
		}
	}

	// Nút Back
	bool backSelected = (selectedSlot == caro::config::MAX_SAVE_SLOTS);
	backButtonShape.setFillColor(backSelected ? sf::Color(80, 30, 120, 220) : sf::Color(10, 5, 30, 190));
	backButtonShape.setOutlineColor(backSelected ? sf::Color(255, 220, 80) : sf::Color(160, 80, 255));
	backButtonText.setFillColor(backSelected ? sf::Color::Yellow : sf::Color::White);
	window.draw(backButtonShape);
	window.draw(backButtonText);
}

void handleLoadScreenInput(sf::Event& event) {
	if (event.type != sf::Event::KeyPressed) return;

	if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W) {
		selectedSlot--;
		if (selectedSlot < 0) selectedSlot = caro::config::MAX_SAVE_SLOTS;
		hoverSound.play();
	}
	else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S) {
		selectedSlot++;
		if (selectedSlot > caro::config::MAX_SAVE_SLOTS) selectedSlot = 0;
		hoverSound.play();
	}
	else if (event.key.code == sf::Keyboard::Enter) {
		selectSound.play();
		if (selectedSlot == caro::config::MAX_SAVE_SLOTS) {
			currentScreen = 0;
		}
		else if (!saveSlots[selectedSlot].isEmpty) {
			sf::sleep(sf::milliseconds(200));
			window.setVisible(false);   // ẩn cửa sổ SFML
			bgMusic.pause();
			caro::GameSession game;
			if (ShowLoadMenu(game)) RunGameLoop(game);
			bgMusic.play();
			window.setVisible(true);    // hiện lại
			currentScreen = 0;          // về menu
			selectedSlot = 0;
		}
	}
	else if (event.key.code == sf::Keyboard::Escape) {
		currentScreen = 0;
	}
}

string getAIDifficultyString(caro::AIDifficulty diff) {
	switch (diff) {
	case caro::AIDifficulty::Easy:   return "EASY";
	case caro::AIDifficulty::Medium: return "MEDIUM";
	case caro::AIDifficulty::Hard:   return "HARD";
	case caro::AIDifficulty::Master: return "MASTER";
	default: return "EASY";
	}
}

void initSettingsScreen() {
	settingRows.clear();

	// Tiêu đề
	settingsTitleText.setFont(font);
	settingsTitleText.setString("SETTINGS");
	settingsTitleText.setCharacterSize(36);
	sf::FloatRect tr = settingsTitleText.getLocalBounds();
	settingsTitleText.setPosition((WIDTH - tr.width) / 2, 80);

	// Danh sách setting
	vector<pair<string, string>> items = {
		{ "Music Volume", to_string(currentSettings.musicVolume) + " %" },
		{ "Sound Volume", to_string(currentSettings.soundVolume) + " %" },
		{ "AI Difficulty", getAIDifficultyString(currentSettings.aiDifficulty) },
	};

	float startY = 220.f;
	float x = WIDTH / 2 - 320;
	float w = 640, h = 65, cut = 18;

	for (int i = 0; i < (int)items.size(); i++) {
		float y = startY + i * 100;
		SettingRow row;

		// Glow
		row.glow = makeScifiBox(x - 6, y - 6, w + 12, h + 12, cut + 4);
		row.glow.setFillColor(sf::Color(160, 80, 255, 50));

		// Box
		row.box = makeScifiBox(x, y, w, h, cut);
		row.box.setFillColor(sf::Color(10, 5, 30, 190));
		row.box.setOutlineColor(sf::Color(160, 80, 255));
		row.box.setOutlineThickness(2);

		// Label (bên trái)
		row.labelText.setFont(font);
		row.labelText.setString(items[i].first);
		row.labelText.setCharacterSize(18);
		row.labelText.setFillColor(sf::Color(200, 170, 255));
		row.labelText.setPosition(x + cut + 20, y + (h - 18) / 2 - 2);

		// Value (bên phải)
		row.valueText.setFont(font);
		row.valueText.setString(items[i].second);
		row.valueText.setCharacterSize(18);
		row.valueText.setFillColor(sf::Color::White);
		sf::FloatRect vr = row.valueText.getLocalBounds();
		row.valueText.setPosition(x + w - vr.width - 40, y + (h - 18) / 2 - 2);

		settingRows.push_back(row);
	}

	// Nút Back
	settingsBackShape = makeScifiBox(WIDTH / 2 - 100, 600, 200, 50, 14);
	settingsBackShape.setFillColor(sf::Color(10, 5, 30, 190));
	settingsBackShape.setOutlineColor(sf::Color(160, 80, 255));
	settingsBackShape.setOutlineThickness(2);

	settingsBackText.setFont(font);
	settingsBackText.setString("< BACK");
	settingsBackText.setCharacterSize(18);
	settingsBackText.setFillColor(sf::Color::White);
	sf::FloatRect br = settingsBackText.getLocalBounds();
	settingsBackText.setPosition(WIDTH / 2 - br.width / 2, 612);
}

void updateSettingValues() {
	if (settingRows.size() < 3) return;

	// Music Volume
	string musicVal = to_string(currentSettings.musicVolume) + " %";
	settingRows[0].valueText.setString(musicVal);
	sf::FloatRect vr0 = settingRows[0].valueText.getLocalBounds();
	float x = WIDTH / 2 - 320;
	float w = 640;
	settingRows[0].valueText.setPosition(x + w - vr0.width - 40, settingRows[0].valueText.getPosition().y);

	// Sound Volume
	string soundVal = to_string(currentSettings.soundVolume) + " %";
	settingRows[1].valueText.setString(soundVal);
	sf::FloatRect vr1 = settingRows[1].valueText.getLocalBounds();
	settingRows[1].valueText.setPosition(x + w - vr1.width - 40, settingRows[1].valueText.getPosition().y);

	// AI Difficulty
	settingRows[2].valueText.setString(getAIDifficultyString(currentSettings.aiDifficulty));
	sf::FloatRect vr2 = settingRows[2].valueText.getLocalBounds();
	settingRows[2].valueText.setPosition(x + w - vr2.width - 40, settingRows[2].valueText.getPosition().y);

	// Áp dụng âm lượng thực tế
	bgMusic.setVolume((float)currentSettings.musicVolume);
	hoverSound.setVolume((float)currentSettings.soundVolume);
	selectSound.setVolume((float)currentSettings.soundVolume);
}

void drawSettingsScreen() {
	float t = menuClock.getElapsedTime().asSeconds();
	float pulse = (sin(t * 3.0f) + 1.0f) / 2.0f;

	// Tiêu đề gradient
	float r1 = (sin(t * 1.2f) + 1.f) / 2.f;
	float r2 = (sin(t * 1.2f + 2.1f) + 1.f) / 2.f;
	float r3 = (sin(t * 1.2f + 4.2f) + 1.f) / 2.f;
	sf::Color titleColor(
		(sf::Uint8)(180 + r1 * 75),
		(sf::Uint8)(80 + r2 * 100),
		(sf::Uint8)(200 + r3 * 55)
	);
	settingsTitleText.setFillColor(titleColor);

	int offsets[8][2] = {
		{-3,0},{3,0},{0,-3},{0,3},
		{-3,-3},{3,-3},{-3,3},{3,3}
	};
	sf::Text titleOutline = settingsTitleText;
	titleOutline.setFillColor(sf::Color(20, 0, 50, 200));
	for (auto& o : offsets) {
		titleOutline.setPosition(
			settingsTitleText.getPosition().x + o[0],
			settingsTitleText.getPosition().y + o[1]
		);
		window.draw(titleOutline);
	}
	window.draw(settingsTitleText);

	// Hint điều hướng
	sf::Text hint;
	hint.setFont(font);
	hint.setString("< >  de thay doi    W/S  de chon");
	hint.setCharacterSize(11);
	hint.setFillColor(sf::Color(150, 130, 200, 180));
	sf::FloatRect hr = hint.getLocalBounds();
	hint.setPosition((WIDTH - hr.width) / 2, 150);
	window.draw(hint);

	// Các row setting
	for (int i = 0; i < (int)settingRows.size(); i++) {
		bool isSelected = (i == selectedSetting);
		if (isSelected) {
			sf::Uint8 r = (sf::Uint8)(60 + pulse * 80);
			sf::Uint8 g = (sf::Uint8)(10 + pulse * 40);
			sf::Uint8 b = (sf::Uint8)(160 + pulse * 80);
			settingRows[i].box.setFillColor(sf::Color(r, g, b, 220));
			settingRows[i].box.setOutlineColor(sf::Color(255, 220, 80));
			settingRows[i].box.setOutlineThickness(3);
			settingRows[i].labelText.setFillColor(sf::Color::Yellow);
			settingRows[i].valueText.setFillColor(sf::Color(255, 220, 80));

			sf::Uint8 glowA = (sf::Uint8)(40 + pulse * 80);
			settingRows[i].glow.setFillColor(sf::Color(180, 80, 255, glowA));
			window.draw(settingRows[i].glow);

			// Arrow trái phải gợi ý
			sf::Text arrowL, arrowR;
			arrowL.setFont(font); arrowL.setString("<");
			arrowR.setFont(font); arrowR.setString(">");
			arrowL.setCharacterSize(16);
			arrowR.setCharacterSize(16);
			sf::Uint8 aa = (sf::Uint8)(150 + pulse * 105);
			arrowL.setFillColor(sf::Color(255, 220, 80, aa));
			arrowR.setFillColor(sf::Color(255, 220, 80, aa));

			float boxX = settingRows[i].box.getPoint(3).x;
			float boxY = settingRows[i].box.getPoint(3).y;
			float boxR = settingRows[i].box.getPoint(4).x;
			arrowL.setPosition(boxX - 25, boxY + 18);
			arrowR.setPosition(boxR + 8, boxY + 18);

			window.draw(settingRows[i].box);
			window.draw(settingRows[i].labelText);
			window.draw(settingRows[i].valueText);
			window.draw(arrowL);
			window.draw(arrowR);
		}
		else {
			settingRows[i].box.setFillColor(sf::Color(10, 5, 30, 190));
			settingRows[i].box.setOutlineColor(sf::Color(120, 60, 200, 180));
			settingRows[i].box.setOutlineThickness(2);
			settingRows[i].labelText.setFillColor(sf::Color(200, 170, 255));
			settingRows[i].valueText.setFillColor(sf::Color::White);
			window.draw(settingRows[i].box);
			window.draw(settingRows[i].labelText);
			window.draw(settingRows[i].valueText);
		}
	}

	// Nút Back
	bool backSel = (selectedSetting == (int)settingRows.size());
	settingsBackShape.setFillColor(backSel ? sf::Color(80, 30, 120, 220) : sf::Color(10, 5, 30, 190));
	settingsBackShape.setOutlineColor(backSel ? sf::Color(255, 220, 80) : sf::Color(160, 80, 255));
	settingsBackText.setFillColor(backSel ? sf::Color::Yellow : sf::Color::White);
	window.draw(settingsBackShape);
	window.draw(settingsBackText);
}

void handleSettingsInput(sf::Event& event) {
	if (event.type != sf::Event::KeyPressed) return;

	int maxRow = (int)settingRows.size(); // Back = index maxRow

	if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W) {
		selectedSetting--;
		if (selectedSetting < 0) selectedSetting = maxRow;
		hoverSound.play();
	}
	else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S) {
		selectedSetting++;
		if (selectedSetting > maxRow) selectedSetting = 0;
		hoverSound.play();
	}
	else if (event.key.code == sf::Keyboard::Left ||
		event.key.code == sf::Keyboard::A) {
		if (selectedSetting == 0) { // Music Volume
			currentSettings.musicVolume -= 10;
			if (currentSettings.musicVolume < 0) currentSettings.musicVolume = 0;
		}
		else if (selectedSetting == 1) { // Sound Volume
			currentSettings.soundVolume -= 10;
			if (currentSettings.soundVolume < 0) currentSettings.soundVolume = 0;
		}
		else if (selectedSetting == 2) { // AI Difficulty
			int d = (int)currentSettings.aiDifficulty - 1;
			if (d < 0) d = 3;
			currentSettings.aiDifficulty = (caro::AIDifficulty)d;
		}
		updateSettingValues();
		hoverSound.play();
	}
	else if (event.key.code == sf::Keyboard::Right ||
		event.key.code == sf::Keyboard::D) {
		if (selectedSetting == 0) { // Music Volume
			currentSettings.musicVolume += 10;
			if (currentSettings.musicVolume > 100) currentSettings.musicVolume = 100;
		}
		else if (selectedSetting == 1) { // Sound Volume
			currentSettings.soundVolume += 10;
			if (currentSettings.soundVolume > 100) currentSettings.soundVolume = 100;
		}
		else if (selectedSetting == 2) { // AI Difficulty
			int d = (int)currentSettings.aiDifficulty + 1;
			if (d > 3) d = 0;
			currentSettings.aiDifficulty = (caro::AIDifficulty)d;
		}
		updateSettingValues();
		hoverSound.play();
	}
	else if (event.key.code == sf::Keyboard::Enter) {
		if (selectedSetting == maxRow) {
			selectSound.play();
			currentScreen = 0;
		}
	}
	else if (event.key.code == sf::Keyboard::Escape) {
		currentScreen = 0;
	}
}

// =============================================
// MAIN
// =============================================
int main() {
	string assetsPath = getAssetsPath();

	if (!font.loadFromFile(assetsPath + "/font/PressStart2P-Regular.ttf")) {
		cout << "Cannot load font\n";
		return -1;
	}
	if (!menuBgTexture.loadFromFile(assetsPath + "/background3.jpg"))
		cout << "Cannot load background\n";

	if (!hoverBuffer.loadFromFile(assetsPath + "/music/move.wav"))
		cout << "Cannot load hover sound\n";
	hoverSound.setBuffer(hoverBuffer);
	hoverSound.setVolume(70.f);

	if (!selectBuffer.loadFromFile(assetsPath + "/music/selected.wav"))
		cout << "Cannot load select sound\n";
	selectSound.setBuffer(selectBuffer);
	selectSound.setVolume(80.f);

	bgMusic.openFromFile(assetsPath + "/music/sound.mp3");
	bgMusic.setLoop(true);
	bgMusic.setVolume(50.f);
	bgMusic.play();

	menuBgSprite.setColor(sf::Color(255, 255, 255, 200));
	menuBgSprite.setTexture(menuBgTexture);
	menuBgSprite.setScale(
		(float)WIDTH / menuBgTexture.getSize().x,
		(float)HEIGHT / menuBgTexture.getSize().y
	);

	initMenu();
	initTitle();
	initParticles();
	currentSettings = caro::CreateDefaultSettings();
	initSettingsScreen();

	while (window.isOpen()) {
		// 1. XỬ LÝ EVENT
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();

			if (currentScreen == 0) handleMenuInput(event);
			if (currentScreen == 2) handleSettingsInput(event);
			if (currentScreen == 3) handleLoadScreenInput(event);
		}

		// 2. VẼ
		window.clear();
		window.draw(menuBgSprite);

		if (currentScreen == 0) {
			updateAndDrawParticles();
			drawTitle();
			drawMenu();
		}
		else if (currentScreen == 2) {              // ← thêm
			updateAndDrawParticles();
			drawSettingsScreen();
		}
		else if (currentScreen == 3) {
			updateAndDrawParticles();
			drawLoadScreen();
		}

		window.display();
	}

	return 0;
}