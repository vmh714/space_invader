#include <gui/playscreen_screen/PlayScreenView.hpp>
#include <BitmapDatabase.hpp>

#define BITMAP_BULLET_ID BITMAP_PLAYER_BULLET_ID
#define ANIM_EXPLOSION_START  BITMAP_EXPLOSION_01_ID
#define ANIM_EXPLOSION_END    BITMAP_EXPLOSION_02_ID
PlayScreenView::PlayScreenView() :
		fireTimer(0), // Khởi tạo timer
		explosionEndedCallback(this, &PlayScreenView::explosionEndedHandler)
{
	// Khởi tạo trạng thái đạn
	for (uint8_t i = 0; i < MAX_BULLETS; ++i)
	{
		bulletStates[i].isActive = false;
		// Không cần khởi tạo x,y bây giờ vì khi bắn sẽ set lại
	}
}
//PlayScreenView::PlayScreenView()
//{
//
//}

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

			explosions[r][c].setXY(enemies[r][c].x, enemies[r][c].y);
			explosions[r][c].setBitmaps(BITMAP_EXPLOSION_01_ID,
					BITMAP_EXPLOSION_02_ID);

			//Tốc độ: cứ 5 ticks đổi 1 hình (tăng nếu muốn nổ chậm hơn)
			explosions[r][c].setUpdateTicksInterval(5);

			// Đăng ký callback khi kết thúc
			explosions[r][c].setDoneAction(explosionEndedCallback);

			explosions[r][c].setVisible(false);
			add (explosions[r][c]);
		}
	}

	// --- SETUP ĐẠN PLAYER (MỚI) ---
	for (uint8_t i = 0; i < MAX_BULLETS; ++i)
	{
		bulletImages[i].setBitmap(Bitmap(BITMAP_BULLET_ID));
		bulletImages[i].setVisible(false); // Ban đầu ẩn đi
		// Thêm vào view. Lưu ý Z-order: add sau sẽ đè lên cái add trước.
		// Player được add ở ViewBase. Nếu muốn đạn bay "dưới" player thì phải add thủ thuật khác.
		// Add ở đây thì đạn sẽ bay "trên" player.
		add (bulletImages[i]);
	}

	// Reset timer khi vào màn hình
	fireTimer = 0;

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
// --- CÁC HÀM MỚI CHO VIỆC BẮN ĐẠN ---

void PlayScreenView::handleTickEvent()
{
	// Hàm này được gọi mỗi khung hình (ví dụ 60 lần/giây)

	// 1. Tăng bộ đếm thời gian bắn
	fireTimer++;

	// 2. Kiểm tra điều kiện để bắn viên đạn mới (Auto fire)
	if (fireTimer >= FIRE_DELAY_TICKS)
	{
		spawnBullet();
		fireTimer = 0; // Reset timer sau khi thử bắn
	}

	// 3. Cập nhật vị trí các viên đạn đang bay
	updateBullets();

	checkCollisions();
}

void PlayScreenView::spawnBullet()
{
	// Tìm một slot đạn đang rảnh
	for (uint8_t i = 0; i < MAX_BULLETS; ++i)
	{
		if (!bulletStates[i].isActive)
		{
			// Tính toán vị trí xuất phát (như bạn yêu cầu)
			// Player 40x40. Bullet 6x12.
			// Center X player = player.getX() + 20
			// Bullet X = Center X player - (BulletWidth / 2) = player.getX() + 20 - 3 = +17
			// Bullet Y = player.getY() - BulletHeight = player.getY() - 12

			bulletStates[i].x = player.getX() + 17;
			bulletStates[i].y = player.getY() - 12;
			bulletStates[i].isActive = true;

			// Cập nhật hình ảnh
			bulletImages[i].setXY(bulletStates[i].x, bulletStates[i].y);
			bulletImages[i].setVisible(true);
			bulletImages[i].invalidate(); // Vẽ lại viên đạn

			// Chỉ bắn 1 viên mỗi lần gọi hàm này
			return;
		}
	}
	// Nếu chạy đến đây nghĩa là đã đủ 2 viên đạn, không bắn thêm.
}

void PlayScreenView::updateBullets()
{
	for (uint8_t i = 0; i < MAX_BULLETS; ++i)
	{
		if (bulletStates[i].isActive)
		{
			// Bay lên trên (trừ Y)
			bulletStates[i].y -= BULLET_SPEED;

			// Kiểm tra nếu bay ra khỏi cạnh trên màn hình
			// Màn hình cao 320, tọa độ 0 ở trên cùng.
			// Khi Y < -BULLET_HEIGHT nghĩa là đã khuất hoàn toàn.
			if (bulletStates[i].y < -BULLET_HEIGHT)
			{
				bulletStates[i].isActive = false;
				bulletImages[i].setVisible(false);
				bulletImages[i].invalidate();
			}
			else
			{
				// Di chuyển hình ảnh đến vị trí mới
				// Dùng moveTo sẽ tự động invalidate vùng cũ và mới
				bulletImages[i].moveTo(bulletStates[i].x, bulletStates[i].y);
			}
		}
	}
}
void PlayScreenView::checkCollisions()
{
	// Kích thước Enemy (dựa vào code setup của bạn là 32x32)
	const uint16_t ENEMY_SIZE = 32;

	for (uint8_t i = 0; i < MAX_BULLETS; ++i)
	{
		// Chỉ kiểm tra đạn đang bay
		if (!bulletStates[i].isActive)
			continue;

		// Tạo Rect cho viên đạn
		// Tọa độ bulletStates là top-left
		int16_t bX = bulletStates[i].x;
		int16_t bY = bulletStates[i].y;
		int16_t bW = BULLET_WIDTH;
		int16_t bH = BULLET_HEIGHT;

		// Duyệt qua tất cả kẻ địch
		for (uint8_t r = 0; r < ROWS; r++)
		{
			for (uint8_t c = 0; c < COLS; c++)
			{
				// Chỉ kiểm tra kẻ địch còn sống
				if (!enemies[r][c].alive)
					continue;

				int16_t eX = enemies[r][c].x;
				int16_t eY = enemies[r][c].y;
				int16_t eW = ENEMY_SIZE;
				int16_t eH = ENEMY_SIZE;

				// --- AABB COLLISION DETECTION (Thuật toán va chạm hộp chữ nhật) ---
				// Điều kiện va chạm:
				// bLeft < eRight && bRight > eLeft && bTop < eBottom && bBottom > eTop
				if (bX < eX + eW && bX + bW > eX && bY < eY + eH
						&& bY + bH > eY)
				{
					// === ĐÃ BẮN TRÚNG ===

					// 1. Xử lý Đạn: Ẩn và Reset
					bulletStates[i].isActive = false;
					bulletImages[i].setVisible(false);
					bulletImages[i].invalidate();

					// 2. Xử lý Enemy: Đánh dấu chết và Ẩn
					enemies[r][c].alive = false;
					enemyImages[r][c].setVisible(false);
					enemyImages[r][c].invalidate();

					// 3. Kích hoạt hiệu ứng Nổ
					explosions[r][c].setVisible(true);
					explosions[r][c].startAnimation(false, false, false); // start, no loop, no reverse
					explosions[r][c].invalidate();

					// Break loop enemy (Một viên đạn chỉ trúng 1 enemy tại 1 thời điểm)
					goto next_bullet;
				}
			}
		}
		next_bullet: ;
	}
}

// Callback được gọi tự động khi AnimatedImage chạy xong frame cuối
void PlayScreenView::explosionEndedHandler(const touchgfx::AnimatedImage& source)
{
    // 1. Chuyển đổi từ const sang đối tượng có thể chỉnh sửa
    touchgfx::AnimatedImage& anim = const_cast<touchgfx::AnimatedImage&>(source);

    // 2. Ẩn ảnh vụ nổ
    anim.setVisible(false);

    // 3. Quan trọng: Yêu cầu vẽ lại vùng này để xóa ảnh cũ khỏi màn hình
    anim.invalidate();
}
