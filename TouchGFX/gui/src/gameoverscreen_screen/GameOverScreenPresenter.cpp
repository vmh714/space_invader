#include <gui/gameoverscreen_screen/GameOverScreenView.hpp>
#include <gui/gameoverscreen_screen/GameOverScreenPresenter.hpp>

GameOverScreenPresenter::GameOverScreenPresenter(GameOverScreenView &v) :
		view(v)
{

}

void GameOverScreenPresenter::activate()
{
	// Lấy điểm từ Model
	uint32_t finalScore = model->getFinalScore();

	// Đẩy điểm số sang View để hiển thị lên màn hình
	view.displayScore(finalScore);
}

void GameOverScreenPresenter::deactivate()
{

}
