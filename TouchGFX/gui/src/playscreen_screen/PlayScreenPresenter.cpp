#include <gui/playscreen_screen/PlayScreenView.hpp>
#include <gui/playscreen_screen/PlayScreenPresenter.hpp>
#include <gui/common/FrontendApplication.hpp>
#include <stdint.h>

PlayScreenPresenter::PlayScreenPresenter(PlayScreenView &v) :
		view(v)
{

}

void PlayScreenPresenter::activate()
{

}

void PlayScreenPresenter::deactivate()
{

}
void PlayScreenPresenter::movePlayer(char direction)
{
	view.movePlayer(direction);
}
void PlayScreenPresenter::goToGameOverScreen()
{
	uint32_t score = view.getCurrentScore();
	model->setFinalScore(score);
	view.changeToGameOver();
}
