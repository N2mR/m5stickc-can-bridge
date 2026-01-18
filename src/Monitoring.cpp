#include "Monitoring.h"


Monitoring::Monitoring(CAN_device_t& CAN_cfg)
    :m_CAN_cfg(CAN_cfg)
{
}

/**
 * @details rx_queueに応じてtrue/falseを返す。trueの場合は呼び出し元でrx_frameを使用する
*/
bool Monitoring::getCANData(CAN_frame_t& rx_frame)
{
	return xQueueReceive(m_CAN_cfg.rx_queue, &rx_frame, 3*portTICK_PERIOD_MS) == pdTRUE;
}
