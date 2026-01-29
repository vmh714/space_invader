#include <gui/gameoverscreen_screen/GameOverScreenView.hpp>

GameOverScreenView::GameOverScreenView()
{

}

void GameOverScreenView::setupScreen()
{
    GameOverScreenViewBase::setupScreen();
//    Unicode::snprintf(scoreTextBuffer, SCORETEXT_SIZE, "%d", 0);
}

void GameOverScreenView::tearDownScreen()
{
    GameOverScreenViewBase::tearDownScreen();
}
void GameOverScreenView::displayScore(uint32_t score)
{
    // Chuyển số thành chuỗi Unicode và hiển thị lên TextArea scoreText
    Unicode::snprintf(scoreTextBuffer, SCORETEXT_SIZE, "%d", score);
    scoreText.invalidate();
}
