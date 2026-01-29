#ifndef GAMEOVERSCREENPRESENTER_HPP
#define GAMEOVERSCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class GameOverScreenView;

class GameOverScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    GameOverScreenPresenter(GameOverScreenView& v);

    /**
     * The activate function is called automatically when this screen is "switched in"
     * (ie. made active). Initialization logic can be placed here.
     */
    virtual void activate();

    /**
     * The deactivate function is called automatically when this screen is "switched out"
     * (ie. made inactive). Teardown functionality can be placed here.
     */
    virtual void deactivate();

    virtual ~GameOverScreenPresenter() {}

private:
    GameOverScreenPresenter();

    GameOverScreenView& view;
};

#endif // GAMEOVERSCREENPRESENTER_HPP
