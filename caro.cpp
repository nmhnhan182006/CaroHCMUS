#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
using namespace std;

const int WIDTH = 1280;
const int HEIGHT = 720;
const int ROWS = 15;
const int COLS = 15;
const float CELL = 40.f;

sf::RenderWindow window(sf::VideoMode({ WIDTH, HEIGHT }), "Caro Game");

// màn hình
int currentScreen = 0;
// 0 = menu, 1 = playing, 2 = settings, 3 = exit

// menu
vector<string> menuItems = { "New Game", "Settings", "Exit" };
int selectedMenu = 0;

// bàn cờ
int board[ROWS][COLS];
int currentPlayer = 1; // 1 = X, 2 = O

// asset
sf::Font font;
sf::Texture menuBgTexture;
sf::Sprite menuBgSprite;

sf::Texture xTexture;
sf::Texture oTexture;