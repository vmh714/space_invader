#include <gui/gameoverscreen_screen/GameOverScreenView.hpp>
#include <gui/gameoverscreen_screen/GameOverScreenPresenter.hpp>

GameOverScreenPresenter::GameOverScreenPresenter(GameOverScreenView &v) :
		view(v)
{

}

void GameOverScreenPresenter::activate()
{
	// 1. Lấy điểm hiện tại từ Model
	uint32_t finalScore = model->getFinalScore();

	// 2. Yêu cầu Model check Flash và lấy HighScore (có thể tốn 1-2s nếu phải xóa Flash)
	uint32_t highScore = model->updateAndGetHighScore();

	// 3. Đẩy cả 2 điểm sang View
	view.displayScore(finalScore);
	view.displayHighScore(highScore); // Hàm mới bên View
}

void GameOverScreenPresenter::deactivate()
{

}
