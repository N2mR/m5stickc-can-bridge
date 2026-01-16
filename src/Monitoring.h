#ifndef MONITORING_SERVICE_H
#define MONITORING_SERVICE_H

#include <bitset>
#include <vector>

#include "CAN_config.h"
#include "ESP32CAN.h"

#include "CANID.h"
#include "Parser.h"

class Monitoring
{
public:
    Monitoring();
    Monitoring(CAN_device_t& CAN_cfg);
    std::string monitoringCanData();

private:
    bool existsCanFrame();
    bool makeInstance();
    std::string getCanData();
    std::shared_ptr<Parser> m_pParser;
    CAN_device_t& m_CAN_cfg;
};
#endif /* MONITORING_SERVICE_H */