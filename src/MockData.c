#include "MockData.h"

static const Can256Mock can256_mockTable[] = {
    {256, 8, {0x00, 0x00, 0x00, 0x00, 0, 0, 0, 0}}, // Key ON
    {256, 8, {0x80, 0x01, 0x02, 0x00, 0, 0, 0, 0}}, // Cranking
    {256, 8, {0x20, 0x0B, 0x08, 0x00, 0, 0, 0, 0}}, // Idle
    {256, 8, {0x40, 0x0F, 0x20, 0x00, 0, 0, 0, 0}}, // Throttle
    {256, 8, {0x80, 0x17, 0x60, 0x00, 0, 0, 0, 0}}, // Mid
    {256, 8, {0x00, 0x1F, 0xA0, 0x00, 0, 0, 0, 0}}, // High
    {256, 8, {0x40, 0x0C, 0x00, 0x00, 0, 0, 0, 0}}, // Off
    {256, 8, {0x00, 0x00, 0x00, 0x00, 0, 0, 0, 0}}, // Stop
};

static size_t index = 0;
static const size_t table_size = sizeof(can256_mockTable) / sizeof(can256_mockTable[0]);

// CANID256のMockデータをテーブルから取得する
const Can256Mock* CanMock250_GetNext(void)
{
    const Can256Mock* mockData = &can256_mockTable[index];
    index++;

    if (table_size <= index)
    { 
        index = 0;
    }

    return mockData;
}
