#ifndef PLAYSCREENPRESENTER_HPP
#define PLAYSCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class PlayScreenView;

class PlayScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    PlayScreenPresenter(PlayScreenView& v);

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

    virtual void movePlayer(char direction);

    virtual ~PlayScreenPresenter() {}

private:
    PlayScreenPresenter();

    PlayScreenView& view;
};

#endif // PLAYSCREENPRESENTER_HPP
