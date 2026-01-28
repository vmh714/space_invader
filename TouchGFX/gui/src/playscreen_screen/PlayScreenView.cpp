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
            add(enemyImages[r][c]);
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
