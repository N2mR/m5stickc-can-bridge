#ifndef MONITORING_SERVICE_H
#define MONITORING_SERVICE_H

#include <bitset>
#include <vector>

#include "CAN_config.h"
#include "ESP32CAN.h"

#include "CANID.h"
class Monitoring
{
public:
    Monitoring();
    Monitoring(CAN_device_t& CAN_cfg);
    bool getCANData(CAN_frame_t& rx_frame);

private:
    CAN_device_t& m_CAN_cfg;
};
#endif /* MONITORING_SERVICE_H */