#include <M5StickC.h>

#include "CAN_config.h"
#include "ESP32CAN.h"
#include "BluetoothSerial.h"

#include "Monitoring.h"

// M5StickC plusのGroveポート
#define TX_PORT GPIO_NUM_32 
#define RX_PORT GPIO_NUM_33

// Bluetooth
BluetoothSerial SerialBT;
CAN_device_t CAN_cfg;
Monitoring* objMonitoring;
uint16_t tick = 0;

// プロトタイプ宣言
bool existsCanFrame();
std::string getCanData();

void setup() {
	// Initialize the M5StickC objects
	Serial.begin(9600);
	SerialBT.begin("M5StickC");
	M5.begin();

	// 画面の向き(0, 1, 2, 3)
	M5.Lcd.setRotation(1);	
	// テキストサイズ(1-7)
	M5.Lcd.setTextSize(2);

	// CanModule設定
	CAN_cfg.speed = CAN_SPEED_500KBPS;
	CAN_cfg.tx_pin_id = TX_PORT;
	CAN_cfg.rx_pin_id = RX_PORT;
	CAN_cfg.rx_queue = xQueueCreate(10,sizeof(CAN_frame_t));
	ESP32Can.CANInit();

	// CanModuleをMonitoringクラスに渡す
	objMonitoring = new Monitoring(CAN_cfg);
}

void loop() {
	tick++;
	if (tick % 1000 == 0)
	{
		// BluetoothでM5Stackに送信
		std::string strCanData = objMonitoring->monitoringCanData();

		if (!strCanData.empty())
		{
			SerialBT.println(strCanData.c_str());
		}		
	}
}
