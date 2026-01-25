#include <M5StickC.h>

#include "CAN_config.h"
#include "ESP32CAN.h"
#include "BluetoothSerial.h"

#include "Monitoring.h"
#include "MockData.h"

// M5StickC plusのGroveポート
#define TX_PORT GPIO_NUM_32 
#define RX_PORT GPIO_NUM_33

// 制御モード
enum class Mode {
	Bridge,
	Debug
};
// CANフレームの同期用
#define CAN_PKT_MAGIC 0xA5
// CANフレームをBLEで送信する際にQueueで管理するための構造体
typedef struct __attribute__((packed))
{
	uint8_t magic;
	uint32_t id;
	uint8_t dlc;
	uint8_t data[8];
} CanBlePacket;

// Bluetooth
BluetoothSerial SerialBT;
// CANデータフレーム定義
CAN_device_t CAN_cfg;
// CAN監視クラス
Monitoring* objMonitoring;
// ボタン押下ハンドラ
TaskHandle_t buttonTaskHandle = NULL;
// CAN通信確立の判定に使用
static uint32_t nLastCanRxTime = 0;
// CAN通信の確立フラグ
bool blCanConnected = false;

// プロトタイプ宣言
bool existsCanFrame();
bool getCanData(CAN_frame_t& rx_frame);
void bleSendTask();

Mode enmMode;

SemaphoreHandle_t modeMutex;

// ボタン押下タスク
void buttonTask(void *arg) 
{
	bool lastState = HIGH;
	while (1)
	{
		bool currentState = digitalRead(M5_BUTTON_HOME);
		if (lastState == HIGH && currentState == LOW)
		{
			xSemaphoreTake(modeMutex, portMAX_DELAY);
			// Homeボタン押下でModeを切り替える
			if (enmMode == Mode::Bridge)
			{
				enmMode = Mode::Debug;
			} else {
				enmMode = Mode::Bridge;
			}
			xSemaphoreGive(modeMutex);
		}
		lastState = currentState;
		// チャタリング対策
		vTaskDelay(pdMS_TO_TICKS(20));
	}
}

// 実行中のモードを表示
void drawMode(Mode mode)
{
	M5.Lcd.setTextSize(2);
	M5.Lcd.setTextColor(WHITE, TFT_BLACK);
	M5.Lcd.setCursor(2, 2);
	
	const char* strMode;
	if (mode == Mode::Bridge) {
		strMode = "Bridge";
	} else {
		strMode = "Debug";
	}
	M5.Lcd.print(strMode);
}

// 制御状態の表示
void drawState(Mode mode)
{
	M5.Lcd.setTextSize(2);
	M5.Lcd.setTextColor(GREEN, TFT_BLACK);

	char message1[32];
	char message2[32];
	if (mode == Mode::Bridge)
	{
		snprintf(message1, sizeof(message1), "M5Stack: %s", (SerialBT.hasClient()) ? "OK" : "NG");
		snprintf(message2, sizeof(message2), "CAN: %s", (blCanConnected) ? "OK" : "NG");
	} else if (mode == Mode::Debug)
	{
		snprintf(message1, sizeof(message1), "M5Stack: %s", (SerialBT.hasClient()) ? "OK" : "NG");
		snprintf(message2, sizeof(message2), "CAN: %s", (blCanConnected) ? "OK" : "NG");
	} else 
	{
		// no-op
	}

	// メッセージ表示域(上段)
	int16_t x = 1;
	int16_t y = 30;
	M5.Lcd.setCursor(x, y);
	M5.Lcd.print(message1);
	// メッセージ表示域(下段)
	x = 1;
	y = 60;
	M5.Lcd.setCursor(x, y);
	M5.Lcd.print(message2);
}

// 画面更新タスク
void uiTask(void* arg)
{
	Mode lastMode = Mode::Bridge;
	drawMode(lastMode);
	drawState(lastMode);
	while(1)
	{
		xSemaphoreTake(modeMutex, portMAX_DELAY);
		Mode currentMode = enmMode;
		xSemaphoreGive(modeMutex);

		if (lastMode != currentMode)
		{	
			M5.Lcd.fillScreen(TFT_BLACK);
			drawMode(currentMode);
			drawState(currentMode);
			lastMode = currentMode;
		}
		vTaskDelay(pdMS_TO_TICKS(50));
	}
} 

// CANデータフレームの受信後、M5Stackに送信する
void onCanReceive(CAN_frame_t& rx_frame)
{
	CanBlePacket pkt;
	pkt.magic = CAN_PKT_MAGIC;
	pkt.id = rx_frame.MsgID;
	pkt.dlc = rx_frame.FIR.B.DLC;
	memset(pkt.data, 0, sizeof(pkt.data));
	uint8_t len = min(pkt.dlc, (uint8_t)8);
	memcpy(pkt.data, rx_frame.data.u8, len);
	// M5Stackへ送信
	SerialBT.write((uint8_t*)&pkt, sizeof(pkt));
}

// Bridgeモードタスク
void bridgeTask(void* arg)
{
	TickType_t last = xTaskGetTickCount();
	while (1) 
	{
		xSemaphoreTake(modeMutex, portMAX_DELAY);
		Mode enmCurrentMode = enmMode;
		xSemaphoreGive(modeMutex);
		// Bridgeモードのみ
		if (enmCurrentMode == Mode::Bridge)
		{
			CAN_frame_t rx_frame;
			// CANフレーム受信時
			if (objMonitoring->getCANData(rx_frame))
			{
				blCanConnected = true;
				nLastCanRxTime = millis();
				// M5Stackに送信する
				onCanReceive(rx_frame);
			}
			// 500ms以上フレームを受信できない場合は切断と判定
			if ((millis() - nLastCanRxTime > 500))
			{
				blCanConnected = false;
			}
		}
		vTaskDelayUntil(&last, pdMS_TO_TICKS(5));
	}
}

// CANデータフレームの受信後、M5Stackに送信する
void sendMockData(uint32_t id, uint8_t dlc, uint8_t data[8])
{
	CanBlePacket pkt;
	pkt.magic = CAN_PKT_MAGIC;
	pkt.id = id;
	pkt.dlc = dlc;
	memset(pkt.data, 0, sizeof(pkt.data));
	memcpy(pkt.data, data, sizeof(pkt.data));
	// M5Stackへ送信
	SerialBT.write((uint8_t*)&pkt, sizeof(pkt));
}

// Debugタスク
void debugTask(void* arg)
{
	TickType_t last = xTaskGetTickCount();
	while (1)
	{
		xSemaphoreTake(modeMutex, portMAX_DELAY);
		Mode enmCurrentMode = enmMode;
		xSemaphoreGive(modeMutex);

		// Debugモードのみ
		if (enmCurrentMode == Mode::Debug)
		{
			const Can256Mock *mockData256 = CanMock250_GetNext();
			CanBlePacket pkt256;
			pkt256.magic = CAN_PKT_MAGIC;
			pkt256.id = mockData256->id;
			pkt256.dlc = mockData256->dlc;
			memset(pkt256.data, 0x00, sizeof(pkt256.data));
			memcpy(pkt256.data, mockData256->data, sizeof(pkt256.data));
			
			// シリアル出力
			Serial.print("id=");
			Serial.print(pkt256.id);
			Serial.print("dlc=");
			Serial.print(pkt256.dlc);
			Serial.print("data=");
			for (uint8_t i = 0; i < sizeof(pkt256.data); i++)
			{
				if (pkt256.data[i] < 16) Serial.print("0");
				Serial.print(pkt256.data[i], HEX);
			}
			Serial.println();

			// M5Stackへ送信
			SerialBT.write((uint8_t*)&pkt256, sizeof(pkt256));
		}
		vTaskDelayUntil(&last, pdMS_TO_TICKS(5));
	}
}

void setup()
{
	M5.begin();
	Serial.begin(9600);
	SerialBT.begin("M5StickC");

	// 画面の向き(0, 1, 2, 3)
	M5.Lcd.setRotation(1); // 横向き
	M5.Lcd.fillScreen(TFT_BLACK);

	// CanModule設定
	CAN_cfg.speed = CAN_SPEED_500KBPS;
	CAN_cfg.tx_pin_id = TX_PORT;
	CAN_cfg.rx_pin_id = RX_PORT;
	CAN_cfg.rx_queue = xQueueCreate(10,sizeof(CAN_frame_t));
	ESP32Can.CANInit();

	// CAN通信設定をMonitoringクラスに渡す
	objMonitoring = new Monitoring(CAN_cfg);
	// modeのmutexを作成
	modeMutex = xSemaphoreCreateMutex();

	// 画面更新タスク
	xTaskCreate(uiTask, "uiTask", 4096, NULL, 1, NULL);
	// ボタン押下タスク
	xTaskCreate(buttonTask, "buttonTask", 3072, NULL, 2, &buttonTaskHandle);
	// Bridgeタスク
	xTaskCreate(bridgeTask, "bridgeTask", 4096, NULL, 2, NULL);
	// Debugタスク
	xTaskCreate(debugTask, "debugTask", 4096, NULL, 2, NULL);
	// デフォルトはBridgeモード
	enmMode = Mode::Bridge;
}

void loop()
{
}