#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>

#ifndef SIMULATION
#include <cmsis_os2.h>
extern osMessageQueueId_t controlQueueHandle;
#endif

Model::Model() : modelListener(0)
{

}
void Model::tick()
{
#ifndef SIMULATION
	if (controlQueueHandle != NULL)
	{
		char cmd = 0;
		if (osMessageQueueGet(controlQueueHandle, &cmd, NULL, 0) == osOK)
		{
			modelListener->movePlayer(cmd);
		}
	}
#endif
}
