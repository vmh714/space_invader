#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>

#include "stm32f4xx_hal.h"

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
uint32_t Model::readHighScoreFromFlash()
{
#ifdef SIMULATOR
    return 0; // Giả lập trả về 0
#else
    // Đọc trực tiếp từ địa chỉ bộ nhớ
    uint32_t val = *(__IO uint32_t*)FLASH_USER_START_ADDR;

    // Nếu Flash chưa từng ghi (toàn F), trả về 0
    if (val == 0xFFFFFFFF) {
        return 0;
    }
    return val;
#endif
}

void Model::writeHighScoreToFlash(uint32_t score)
{
#ifndef SIMULATOR
    HAL_FLASH_Unlock();

    // 1. Xóa Sector cũ trước khi ghi (Bắt buộc với Flash STM32)
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t SectorError;

    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    EraseInitStruct.Sector = FLASH_SECTOR_23; // Sector cuối cùng
    EraseInitStruct.NbSectors = 1;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) == HAL_OK)
    {
        // 2. Ghi điểm mới
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_USER_START_ADDR, score);
    }

    HAL_FLASH_Lock();
#endif
}

uint32_t Model::updateAndGetHighScore()
{
    uint32_t currentHigh = readHighScoreFromFlash();

    if (savedScore > currentHigh)
    {
        // Kỷ lục mới! Ghi vào Flash
        writeHighScoreToFlash(savedScore);
        return savedScore;
    }

    return currentHigh;
}
