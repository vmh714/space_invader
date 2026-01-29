#ifndef PLAYSCREENVIEW_HPP
#define PLAYSCREENVIEW_HPP

#include <gui_generated/playscreen_screen/PlayScreenViewBase.hpp>
#include <gui/playscreen_screen/PlayScreenPresenter.hpp>
#include <touchgfx/widgets/Image.hpp>
#include <touchgfx/widgets/AnimatedImage.hpp>

enum EnemyType
{
	ENEMY_BLUE, ENEMY_GREEN
};

struct Enemy
{
	int16_t x;
	int16_t y;
	bool alive;
	EnemyType type;
};
struct PlayerBullet
{
	int16_t x;
	int16_t y;
	bool isActive;
};
struct EnemyBullet
{
	int16_t x, y;
	bool isActive;
};
class PlayScreenView: public PlayScreenViewBase
{
public:
	PlayScreenView();
	virtual ~PlayScreenView()
	{
	}
	virtual void setupScreen();
	virtual void tearDownScreen();

	virtual void killEnemy(uint8_t r, uint8_t c);
	virtual void movePlayer(char direction);

	virtual void handleTickEvent();
	uint32_t getCurrentScore()
	{
		return currentScore;
	}
protected:
	const uint16_t SCREEN_WIDTH = 240;
	const uint16_t SCREEN_HEIGHT = 320;
	static const uint8_t ROWS = 3;
	static const uint8_t COLS = 4;
	// --- PLAYER CONSTANTS ---
	static const uint16_t PLAYER_SIZE = 32;       // Size mới 32x32 của bạn
	static const uint16_t PLAYER_START_X = 104;   // Tọa độ X gốc
	static const uint16_t PLAYER_START_Y = 255;   // Tọa độ Y gốc
	static const uint16_t PLAYER_STEP = 10;       // Tốc độ di chuyển

	// --- BULLET OFFSET (Tọa độ tương đối của đạn so với Player) ---
	// Center X: (32/2) - (6/2) = 13. Y: -12 (bay lên từ đầu player)
	static const int16_t PLAYER_BULLET_OFFSET_X = 13;
	static const int16_t PLAYER_BULLET_OFFSET_Y = -12;
	static const uint8_t MAX_BULLETS = 2; // Số đạn tối đa đồng thời
	static const uint8_t BULLET_WIDTH = 6;
	static const uint8_t BULLET_HEIGHT = 12;
	static const uint8_t BULLET_SPEED = 8; // Tốc độ bay (pixel/frame)
	static const uint8_t FIRE_DELAY_TICKS = 20; // Khoảng cách giữa 2 lần bắn (số frames)

	static const uint8_t MAX_ENEMY_BULLETS = 5; // Số đạn quái tối đa trên màn hình
	static const uint8_t ENEMY_BULLET_SIZE = 8;
	static const uint16_t ENEMY_FIRE_RATE = 100;

	static const uint8_t SCORE_PER_ENEMY = 1;

	uint8_t lastShotColumn;
	Enemy enemies[ROWS][COLS];
	touchgfx::Image enemyImages[ROWS][COLS];

	PlayerBullet bulletStates[MAX_BULLETS]; // Mảng lưu trạng thái logic
	touchgfx::Image bulletImages[MAX_BULLETS]; // Mảng lưu widget hình ảnh
	uint16_t fireTimer; // Bộ đếm thời gian để tự động bắn

	EnemyBullet enemyBulletStates[MAX_ENEMY_BULLETS];
	touchgfx::Image enemyBulletImages[MAX_ENEMY_BULLETS];
	uint16_t enemyFireTimer;

	touchgfx::AnimatedImage explosions[ROWS][COLS];
	// Callback khi animation chạy xong (để ẩn đi)
	touchgfx::Callback<PlayScreenView, const touchgfx::AnimatedImage&> explosionEndedCallback;

	uint16_t respawnTimer;
	uint32_t currentScore;

	void updateBullets();
	void spawnBullet();
	//Hàm kiểm tra va chạm
	void checkCollisions();
	//Hàm xử lý khi animation kết thúc
	void explosionEndedHandler(const touchgfx::AnimatedImage &source);
	void updateEnemyBullets();
	void enemyShoot();
	bool isBottomEnemy(uint8_t r, uint8_t c); // Logic kiểm tra con quái hàng dưới cùng
	void checkPlayerHit(); // Kiểm tra đạn quái trúng người chơi
	void checkAllEnemiesDead();
	void respawnEnemies();
	void updateScoreDisplay();
};

#endif // PLAYSCREENVIEW_HPP
