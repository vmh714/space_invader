#include <gui/playscreen_screen/PlayScreenView.hpp>
#include <BitmapDatabase.hpp>

#define BITMAP_BULLET_ID BITMAP_PLAYER_BULLET_ID
#define ANIM_EXPLOSION_START  BITMAP_EXPLOSION_01_ID
#define ANIM_EXPLOSION_END    BITMAP_EXPLOSION_02_ID
#define BITMAP_MONSTER_BULLET_ID BITMAP_MONSTER_BULLET_ID

PlayScreenView::PlayScreenView() :
		lastShotColumn(0), fireTimer(0), // Khởi tạo timer
		explosionEndedCallback(this, &PlayScreenView::explosionEndedHandler), currentEnemyFireRate(
				100), // Tốc độ bắn mặc định
		currentScore(0), currentLevel(1), moveDir(MOVE_IDLE), moveTimer(0), moveStepCounter(
				0)

{
	currentLevel = 1;
	currentEnemyFireRate = 100;
	moveDir = MOVE_IDLE;
	moveTimer = 0;
	moveStepCounter = 0;
	// Khởi tạo trạng thái đạn
	for (uint8_t i = 0; i < MAX_BULLETS; ++i)
	{
		bulletStates[i].isActive = false;
	}
}

void PlayScreenView::setupScreen()
{
	PlayScreenViewBase::setupScreen();



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

void PlayScreenView::handleTickEvent()
{
	// Hàm này được gọi mỗi khung hình (ví dụ 60 lần/giây)

	// 1. Logic đạn người chơi
	fireTimer++;
	if (fireTimer >= FIRE_DELAY_TICKS)
	{
		spawnBullet();
		fireTimer = 0;
	}

	updateBullets();
	checkCollisions();

	updateEnemyMovement();

	enemyFireTimer++;
	if (enemyFireTimer >= currentEnemyFireRate)
	{
		enemyShoot();
		enemyFireTimer = 0;
	}

	updateEnemyBullets();

	checkPlayerHit();
	checkAllEnemiesDead();
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
	// 1. Tăng Level
	currentLevel++;

	// 2. Thiết lập độ khó theo Level
	if (currentLevel == 2)
	{
		// Level 2: Bắt đầu di chuyển
		moveDir = MOVE_DOWN; // Bắt đầu chu trình: Xuống -> Trái -> Lên -> Phải
		moveStepCounter = 0;
	}
	else if (currentLevel >= 3)
	{
		// Level 3: Di chuyển + Bắn nhanh hơn
		// Reset lại chu trình di chuyển nếu muốn đồng bộ
		moveDir = MOVE_DOWN;
		moveStepCounter = 0;

		// Tăng tốc độ bắn (số tick giảm đi = bắn nhanh hơn)
		// Giới hạn không cho bắn quá nhanh (ví dụ min là 30)
		if (currentEnemyFireRate > 40)
		{
			currentEnemyFireRate -= 20;
		}
	}

	// 3. Hồi sinh quái (Code cũ)
	for (uint8_t r = 0; r < ROWS; r++)
	{
		for (uint8_t c = 0; c < COLS; c++)
		{
			enemies[r][c].alive = true;
			enemyImages[r][c].setVisible(true);
			enemyImages[r][c].invalidate();
		}
	}
}
void PlayScreenView::updateScoreDisplay()
{
	// Xóa buffer cũ và in số mới vào
	Unicode::snprintf(scoreTextBuffer, SCORETEXT_SIZE, "%d", currentScore);

	scoreText.invalidate();
}
void PlayScreenView::updateEnemyMovement()
{
	// Kiểm tra Level và trạng thái IDLE
	if (currentLevel < 2 || moveDir == MOVE_IDLE)
		return;

	// Timer tốc độ di chuyển
	moveTimer++;
	if (moveTimer < MOVE_SPEED_DELAY)
		return;
	moveTimer = 0;

	int16_t dx = 0;
	int16_t dy = 0;

	// 1. Xác định hướng dự kiến
	switch (moveDir)
	{
	case MOVE_DOWN:
		dy = MOVE_PIXELS;
		break;
	case MOVE_LEFT:
		dx = -MOVE_PIXELS;
		break;
	case MOVE_UP:
		dy = -MOVE_PIXELS;
		break;
	case MOVE_RIGHT:
		dx = MOVE_PIXELS;
		break;
	default:
		break;
	}

	// 2. Kiểm tra va chạm
	if (canMove(dx, dy))
	{
		// === TRƯỜNG HỢP A: ĐƯỜNG THOÁNG, CỨ ĐI ===

		// Cập nhật vị trí thật
		for (uint8_t r = 0; r < ROWS; r++)
		{
			for (uint8_t c = 0; c < COLS; c++)
			{
				enemies[r][c].x += dx;
				enemies[r][c].y += dy;
				if (enemies[r][c].alive)
				{
					enemyImages[r][c].moveTo(enemies[r][c].x, enemies[r][c].y);
				}
			}
		}

		// Tăng biến đếm quãng đường
		moveStepCounter += MOVE_PIXELS;

		// Nếu đi đủ quãng đường quy định (MOVE_RANGE) thì đổi hướng
		if (moveStepCounter >= MOVE_RANGE)
		{
			moveStepCounter = 0;
			// Chuyển hướng theo chiều kim đồng hồ
			switch (moveDir)
			{
			case MOVE_DOWN:
				moveDir = MOVE_LEFT;
				break;
			case MOVE_LEFT:
				moveDir = MOVE_UP;
				break;
			case MOVE_UP:
				moveDir = MOVE_RIGHT;
				break;
			case MOVE_RIGHT:
				moveDir = MOVE_DOWN;
				break;
			default:
				break;
			}
		}
	}
	else
	{
		// === TRƯỜNG HỢP B: ĐỤNG TƯỜNG -> ĐỔI HƯỚNG NGAY ===
		// (Đây chính là logic bạn muốn: bắt đầu vòng đời mới ngay)

		moveStepCounter = 0; // Reset quãng đường về 0 để hướng mới đi đủ range

		// Ép buộc chuyển hướng ngay lập tức
		switch (moveDir)
		{
		case MOVE_DOWN:
			moveDir = MOVE_LEFT;
			break;
		case MOVE_LEFT:
			// Đang sang Trái mà đụng tường -> Đi Lên ngay
			moveDir = MOVE_UP;
			break;
		case MOVE_UP:
			moveDir = MOVE_RIGHT;
			break;
		case MOVE_RIGHT:
			// Đang sang Phải mà đụng tường -> Đi Xuống ngay
			moveDir = MOVE_DOWN;
			break;
		default:
			break;
		}
	}
}
bool PlayScreenView::canMove(int16_t dx, int16_t dy)
{
	for (uint8_t r = 0; r < ROWS; r++)
	{
		for (uint8_t c = 0; c < COLS; c++)
		{
			if (enemies[r][c].alive)
			{
				int16_t nextX = enemies[r][c].x + dx;
				int16_t nextY = enemies[r][c].y + dy;

				// Kiểm tra biên màn hình (0 - 240 và 0 - 320)
				// Cộng 32 vì đó là kích thước ảnh (imgSize)
				if (nextX < 0 || nextX + 32 > SCREEN_WIDTH)
					return false;
				if (nextY < 0 || nextY + 32 > SCREEN_HEIGHT)
					return false;
			}
		}
	}
	return true;
}
