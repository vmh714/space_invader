#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>

#ifndef SIMULATOR
#include <cmsis_os2.h>
extern osMessageQueueId_t controlQueueHandle;
#endif

Model::Model() :
		modelListener(0),savedScore(0)
{

}
void Model::tick()
{
#ifndef SIMULATOR
	if (controlQueueHandle != NULL
			&& osMessageQueueGetCount(controlQueueHandle) > 0)
	{
		uint16_t received_cmd;
		if (osMessageQueueGet(controlQueueHandle, &received_cmd, NULL, 0)
				== osOK)
		{
			// Chỉ gửi lệnh đi nếu Listener (Presenter) vẫn còn tồn tại
			if (modelListener != nullptr)
			{
				modelListener->movePlayer((char) received_cmd);
			}
		}
	}
#endif
}
