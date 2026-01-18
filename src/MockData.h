#ifndef MOCK_DATA_H
#define MOCK_DATA_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif
#define CAN_ID_256 0x100

typedef struct  
{
    uint32_t id;
    uint8_t dlc;
    uint8_t data[8];
} Can256Mock;

const Can256Mock* CanMock250_GetNext(void);

#ifdef __cplusplus
}
#endif
#endif