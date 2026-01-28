#ifndef PLAYSCREENVIEW_HPP
#define PLAYSCREENVIEW_HPP

#include <gui_generated/playscreen_screen/PlayScreenViewBase.hpp>
#include <gui/playscreen_screen/PlayScreenPresenter.hpp>
#include <touchgfx/widgets/Image.hpp>

enum EnemyType
{
    ENEMY_BLUE,
    ENEMY_GREEN
};

struct Enemy
{
    int16_t x;
    int16_t y;
    bool alive;
    EnemyType type;
};

class PlayScreenView : public PlayScreenViewBase
{
public:
    PlayScreenView();
    virtual ~PlayScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void killEnemy(uint8_t r, uint8_t c);
protected:
    static const uint8_t ROWS = 3;
	static const uint8_t COLS = 4;

	Enemy enemies[ROWS][COLS];
	touchgfx::Image enemyImages[ROWS][COLS];
};

#endif // PLAYSCREENVIEW_HPP
