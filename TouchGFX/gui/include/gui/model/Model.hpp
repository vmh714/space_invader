#ifndef MODEL_HPP
#define MODEL_HPP
#include <stdint.h>

#define FLASH_USER_START_ADDR   ((uint32_t)0x081E0000)

class ModelListener;

class Model
{
public:
	Model();

	void bind(ModelListener *listener)
	{
		modelListener = listener;
	}

	void tick();
	void setFinalScore(uint32_t score)
	{
		savedScore = score;
	}
	uint32_t getFinalScore()
	{
		return savedScore;
	}
	uint32_t updateAndGetHighScore();
protected:
	ModelListener *modelListener;
	uint32_t savedScore;

	uint32_t readHighScoreFromFlash();
	void writeHighScoreToFlash(uint32_t score);
};

#endif // MODEL_HPP
