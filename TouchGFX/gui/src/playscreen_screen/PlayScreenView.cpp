#include <gui/playscreen_screen/PlayScreenView.hpp>
#include <BitmapDatabase.hpp>
PlayScreenView::PlayScreenView()
{

}

void PlayScreenView::setupScreen()
{
	PlayScreenViewBase::setupScreen();

	const uint16_t imgSize = 32;
	const uint16_t spacingX = 20;
	const uint16_t spacingY = 0;

	const uint16_t startX = 20;
	const uint16_t startY = 20;

	for (uint8_t r = 0; r < ROWS; r++)
	{
		for (uint8_t c = 0; c < COLS; c++)
		{
			enemies[r][c].x = startX + c * (imgSize + spacingX);
			enemies[r][c].y = startY + r * (imgSize + spacingY);
			enemies[r][c].alive = true;
			enemies[r][c].type = ((r + c) % 2 == 0) ? ENEMY_BLUE : ENEMY_GREEN;

			if (enemies[r][c].type == ENEMY_BLUE)
				enemyImages[r][c].setBitmap(Bitmap(BITMAP_BLUE_32_ID));
			else
				enemyImages[r][c].setBitmap(Bitmap(BITMAP_GREEN_32_ID));

			enemyImages[r][c].setXY(enemies[r][c].x, enemies[r][c].y);
			add (enemyImages[r][c]);
		}
	}

}

void PlayScreenView::tearDownScreen()
{
	PlayScreenViewBase::tearDownScreen();
}
void PlayScreenView::killEnemy(uint8_t r, uint8_t c)
{
	if (enemies[r][c].alive)
	{
		enemies[r][c].alive = false;

		enemyImages[r][c].setVisible(false);
		enemyImages[r][c].invalidate();
	}
}
void PlayScreenView::movePlayer(char direction)
{
	// Lấy vị trí hiện tại của player
	int16_t newX = player.getX();
	int16_t newY = player.getY();

	// Tốc độ di chuyển (số pixel mỗi lần nhấn)
	const int16_t step = 5;

	// Kiểm tra lệnh
	switch (direction)
	{
	case 'U': // Up
		newY -= step;
		break;
	case 'D': // Down
		newY += step;
		break;
	case 'L': // Left
		newX -= step;
		break;
	case 'R': // Right
		newX += step;
		break;
	default:
		return; // Không làm gì nếu ký tự lạ
	}

	// (Tùy chọn) Giới hạn biên màn hình để ảnh không chạy ra ngoài
	// Giả sử màn hình 240x320 hoặc 480x272, bạn tự chỉnh số phù hợp
	if (newX < 0)
		newX = 0;
	if (newY < 0)
		newY = 0;
	if (newX > SCREEN_WIDTH - player.getWidth())
		newX = SCREEN_WIDTH - player.getWidth();
	if (newY > SCREEN_HEIGHT - player.getHeight())
		newY = SCREEN_HEIGHT - player.getHeight();

	// Di chuyển player đến vị trí mới
	// moveTo sẽ tự động invalidate vùng cũ và vẽ lại ở vùng mới
	player.moveTo(newX, newY);
}
