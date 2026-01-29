#ifndef MODEL_HPP
#define MODEL_HPP
#include <stdint.h>
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
protected:
	ModelListener *modelListener;
	uint32_t savedScore;
};

#endif // MODEL_HPP
