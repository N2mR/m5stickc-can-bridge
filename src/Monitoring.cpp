#include "Monitoring.h"


Monitoring::Monitoring(CAN_device_t& CAN_cfg)
    :m_pParser(), m_CAN_cfg(CAN_cfg)
{
    makeInstance();
}

std::string Monitoring::monitoringCanData()
{
    std::string strRet = "";
    // CANフレームが存在していればデータを取得する
	if (existsCanFrame())
	{
		// CanDataを文字列取得
		strRet = getCanData();
	}

    return strRet;
}

bool Monitoring::makeInstance()
{
    std::vector<CANID> lstMonitoringTarget = { CANID::RPM };
    m_pParser = std::make_shared<Parser>(lstMonitoringTarget);
    return true;
}

/**
 * @details CanFrameの存在確認
*/
bool Monitoring::existsCanFrame()
{
	CAN_frame_t rx_frame;
	std::string strMsgID = "";
	// メッセージIDが存在する場合はフレームが存在すると判定
	if(xQueueReceive(m_CAN_cfg.rx_queue, &rx_frame, 3*portTICK_PERIOD_MS) == pdTRUE)
	{
		strMsgID = std::to_string(rx_frame.MsgID);
	}

	return !strMsgID.empty();
}

std::string Monitoring::getCanData()
{
	std::string strOutputCanData = "";
	// CanFrame
	CAN_frame_t rx_frame;

	if(xQueueReceive(m_CAN_cfg.rx_queue, &rx_frame, 3*portTICK_PERIOD_MS) == pdTRUE)
	{
		// 受信時間
		std::string strTime = std::to_string(millis());
		// Msg ID
		std::string strMsgID = std::to_string(rx_frame.MsgID);

		// DLC
		uint8_t DLC = rx_frame.FIR.B.DLC;
		std::string strDLC = std::to_string(DLC);

		// CAN data u8
		std::string strData = "";
		for(uint8_t i = 0 ; i < DLC; i++)
		{
			uint8_t data = rx_frame.data.u8[i];
			strData.append(std::to_string(data));
			strData.append(",");
		}

		strOutputCanData += strTime		+ ",";
		strOutputCanData += strMsgID 	+ ",";
		strOutputCanData += strDLC 		+ ",";
		strOutputCanData += strData;
		strOutputCanData += "\n";
		
		return strOutputCanData;
	}
}