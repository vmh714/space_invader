#include <gui/playscreen_screen/PlayScreenView.hpp>
#include <BitmapDatabase.hpp>

#define BITMAP_BULLET_ID BITMAP_PLAYER_BULLET_ID
#define ANIM_EXPLOSION_START  BITMAP_EXPLOSION_01_ID
#define ANIM_EXPLOSION_END    BITMAP_EXPLOSION_02_ID
#define BITMAP_MONSTER_BULLET_ID BITMAP_MONSTER_BULLET_ID

PlayScreenView::PlayScreenView() :
		lastShotColumn(0), fireTimer(0), // Khởi tạo timer
		explosionEndedCallback(this, &PlayScreenView::explosionEndedHandler), currentScore(
				0)

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

	// --- SETUP ĐẠN PLAYER ---
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

	//set up đạn của quái
	for (uint8_t i = 0; i < MAX_ENEMY_BULLETS; i++)
	{
		enemyBulletStates[i].isActive = false;
		enemyBulletImages[i].setBitmap(Bitmap(BITMAP_MONSTER_BULLET_ID)); // ID từ file của bạn
		enemyBulletImages[i].setVisible(false);
		add (enemyBulletImages[i]);
	}
	enemyFireTimer = 0;
	updateScoreDisplay();

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
	int16_t newX = player.getX();
	int16_t newY = player.getY();

	switch (direction)
	{
	case 'U':
		newY -= PLAYER_STEP;
		break;
	case 'D':
		newY += PLAYER_STEP;
		break;
	case 'L':
		newX -= PLAYER_STEP;
		break;
	case 'R':
		newX += PLAYER_STEP;
		break;
	default:
		return;
	}

	// Ràng buộc biên dựa trên hằng số
	if (newX < 0)
		newX = 0;
	if (newY < 0)
		newY = 0;
	if (newX > SCREEN_WIDTH - PLAYER_SIZE)
		newX = SCREEN_WIDTH - PLAYER_SIZE;
	if (newY > SCREEN_HEIGHT - PLAYER_SIZE)
		newY = SCREEN_HEIGHT - PLAYER_SIZE;

	player.moveTo(newX, newY);
}

// --- CÁC HÀM MỚI CHO VIỆC BẮN ĐẠN ---

void PlayScreenView::handleTickEvent()
{
	// Hàm này được gọi mỗi khung hình (ví dụ 60 lần/giây)
	//Logic đạn người chơi
	//Tăng bộ đếm thời gian bắn
	fireTimer++;

	//Kiểm tra điều kiện để bắn viên đạn mới (Auto fire)
	if (fireTimer >= FIRE_DELAY_TICKS)
	{
		spawnBullet();
		fireTimer = 0; // Reset timer sau khi thử bắn
	}

	// Cập nhật vị trí các viên đạn đang bay
	updateBullets();

	checkCollisions();

	// Logic đạn quái
	enemyFireTimer++;
	if (enemyFireTimer >= ENEMY_FIRE_RATE)
	{
		enemyShoot();
		enemyFireTimer = 0;
	}

	updateEnemyBullets();
	checkPlayerHit();
	checkAllEnemiesDead();
	//updateScoreDisplay();
}

void PlayScreenView::spawnBullet()
{
	for (uint8_t i = 0; i < MAX_BULLETS; ++i)
	{
		if (!bulletStates[i].isActive)
		{
			// Tự động căn giữa dựa trên hằng số đã định nghĩa
			bulletStates[i].x = player.getX() + PLAYER_BULLET_OFFSET_X;
			bulletStates[i].y = player.getY() + PLAYER_BULLET_OFFSET_Y;
			bulletStates[i].isActive = true;

			bulletImages[i].setXY(bulletStates[i].x, bulletStates[i].y);
			bulletImages[i].setVisible(true);
			bulletImages[i].invalidate();
			return;
		}
	}
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

					// CỘNG ĐIỂM
					currentScore += SCORE_PER_ENEMY;
					// Cập nhật lên màn hình
					updateScoreDisplay();
					// Break loop enemy (Một viên đạn chỉ trúng 1 enemy tại 1 thời điểm)
					goto next_bullet;
				}
			}
		}
		next_bullet: ;
	}
}

// Callback được gọi tự động khi AnimatedImage chạy xong frame cuối
void PlayScreenView::explosionEndedHandler(
		const touchgfx::AnimatedImage &source)
{
	// 1. Chuyển đổi từ const sang đối tượng có thể chỉnh sửa
	touchgfx::AnimatedImage &anim = const_cast<touchgfx::AnimatedImage&>(source);

	// 2. Ẩn ảnh vụ nổ
	anim.setVisible(false);

	// 3. Quan trọng: Yêu cầu vẽ lại vùng này để xóa ảnh cũ khỏi màn hình
	anim.invalidate();
}
bool PlayScreenView::isBottomEnemy(uint8_t r, uint8_t c)
{
	if (!enemies[r][c].alive)
		return false;

	// Kiểm tra các hàng bên dưới hàng r (r+1 đến ROWS-1)
	for (uint8_t i = r + 1; i < ROWS; i++)
	{
		if (enemies[i][c].alive)
		{
			return false; // Có con quái khác cản địa phía trước
		}
	}
	return true; // Đây là con hàng dưới cùng của cột c
}
void PlayScreenView::enemyShoot()
{
	uint8_t targetRow = 255;
	uint8_t targetCol = 255;
	bool found = false;

	// Duyệt qua các cột bắt đầu từ cột tiếp theo của lần bắn trước
	for (uint8_t i = 1; i <= COLS; i++)
	{
		uint8_t currentCol = (lastShotColumn + i) % COLS;

		// Tìm con quái dưới cùng của cột này
		for (int8_t r = ROWS - 1; r >= 0; r--)
		{
			if (isBottomEnemy(r, currentCol))
			{
				targetRow = r;
				targetCol = currentCol;
				found = true;
				break;
			}
		}

		if (found)
			break; // Tìm thấy quái ở cột này rồi thì dừng lại để bắn
	}

	if (!found)
		return; // Không còn con quái nào cả

	// Cập nhật lại cột vừa bắn để lần sau bắn cột kế tiếp
	lastShotColumn = targetCol;

	// Tìm slot đạn trống và thực hiện bắn
	for (uint8_t i = 0; i < MAX_ENEMY_BULLETS; i++)
	{
		if (!enemyBulletStates[i].isActive)
		{
			enemyBulletStates[i].isActive = true;

			// Tính toán vị trí dựa trên kích thước 32x32 của quái và 8x8 của đạn
			enemyBulletStates[i].x = enemies[targetRow][targetCol].x + 12; // (32-8)/2 = 12
			enemyBulletStates[i].y = enemies[targetRow][targetCol].y + 32;

			enemyBulletImages[i].setXY(enemyBulletStates[i].x,
					enemyBulletStates[i].y);
			enemyBulletImages[i].setVisible(true);
			enemyBulletImages[i].invalidate();
			break;
		}
	}
}
void PlayScreenView::updateEnemyBullets()
{
	for (uint8_t i = 0; i < MAX_ENEMY_BULLETS; i++)
	{
		if (enemyBulletStates[i].isActive)
		{
			enemyBulletStates[i].y += 4; // Tốc độ đạn quái bay xuống (Y tăng)

			if (enemyBulletStates[i].y > SCREEN_HEIGHT)
			{
				enemyBulletStates[i].isActive = false;
				enemyBulletImages[i].setVisible(false);
			}
			else
			{
				enemyBulletImages[i].moveTo(enemyBulletStates[i].x,
						enemyBulletStates[i].y);
			}
		}
	}
}
void PlayScreenView::checkPlayerHit()
{
	for (uint8_t i = 0; i < MAX_ENEMY_BULLETS; i++)
	{
		if (!enemyBulletStates[i].isActive)
			continue;

		// Va chạm giữa đạn quái (8x8) và Player (PLAYER_SIZE x PLAYER_SIZE)
		if (enemyBulletStates[i].x < player.getX() + PLAYER_SIZE
				&& enemyBulletStates[i].x + ENEMY_BULLET_SIZE > player.getX()
				&& enemyBulletStates[i].y < player.getY() + PLAYER_SIZE
				&& enemyBulletStates[i].y + ENEMY_BULLET_SIZE > player.getY())
		{
			enemyBulletStates[i].isActive = false;
			enemyBulletImages[i].setVisible(false);
			enemyBulletImages[i].invalidate();

			// Chuyển sang GameOver thông qua Action đã fix
			presenter->goToGameOverScreen();
			return;
		}
	}
}
void PlayScreenView::checkAllEnemiesDead()
{
	bool anyAlive = false;
	for (uint8_t r = 0; r < ROWS; r++)
	{
		for (uint8_t c = 0; c < COLS; c++)
		{
			if (enemies[r][c].alive)
			{
				anyAlive = true;
				break;
			}
		}
	}

	if (!anyAlive)
	{
		respawnTimer++;
		if (respawnTimer > 120)
		{ // Đợi 120 ticks (~2 giây ở 60 FPS)
			respawnEnemies();
			respawnTimer = 0;
		}
	}
}

void PlayScreenView::respawnEnemies()
{
	// Reset lại trạng thái ban đầu cho từng con quái
	for (uint8_t r = 0; r < ROWS; r++)
	{
		for (uint8_t c = 0; c < COLS; c++)
		{
			enemies[r][c].alive = true;

			// Hiển thị lại hình ảnh
			enemyImages[r][c].setVisible(true);
			enemyImages[r][c].invalidate();

			// (Tùy chọn) Nếu bạn muốn tăng độ khó mỗi lần hồi sinh,
			// có thể giảm ENEMY_FIRE_RATE ở đây.
		}
	}
}
void PlayScreenView::updateScoreDisplay()
{
	// Xóa buffer cũ và in số mới vào
	Unicode::snprintf(scoreTextBuffer, SCORETEXT_SIZE, "%d", currentScore);

	scoreText.invalidate();
}
