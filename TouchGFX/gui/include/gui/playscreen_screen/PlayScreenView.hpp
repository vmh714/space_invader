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
protected:
	const uint16_t SCREEN_WIDTH = 240;
	const uint16_t SCREEN_HEIGHT = 320;
	static const uint8_t ROWS = 3;
	static const uint8_t COLS = 4;
	Enemy enemies[ROWS][COLS];
	touchgfx::Image enemyImages[ROWS][COLS];

	static const uint8_t MAX_BULLETS = 2; // Số đạn tối đa đồng thời
	static const uint8_t BULLET_WIDTH = 6;
	static const uint8_t BULLET_HEIGHT = 12;
	static const uint8_t BULLET_SPEED = 8; // Tốc độ bay (pixel/frame)
	static const uint8_t FIRE_DELAY_TICKS = 20; // Khoảng cách giữa 2 lần bắn (số frames)

	PlayerBullet bulletStates[MAX_BULLETS]; // Mảng lưu trạng thái logic
	touchgfx::Image bulletImages[MAX_BULLETS]; // Mảng lưu widget hình ảnh
	uint16_t fireTimer; // Bộ đếm thời gian để tự động bắn

	// Hàm trợ giúp
	void updateBullets();
	void spawnBullet();

	touchgfx::AnimatedImage explosions[ROWS][COLS];

	// Callback khi animation chạy xong (để ẩn đi)
	touchgfx::Callback<PlayScreenView, const touchgfx::AnimatedImage&> explosionEndedCallback;

	// [NEW] Hàm kiểm tra va chạm
	void checkCollisions();
	// [NEW] Hàm xử lý khi animation kết thúc
	void explosionEndedHandler(const touchgfx::AnimatedImage &source);
};

#endif // PLAYSCREENVIEW_HPP
