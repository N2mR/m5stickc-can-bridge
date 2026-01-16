#ifndef PARSER_H
#define PARSER_H
#include <M5StickC.h>
#include <vector>
#include "CANID.h"


// TBD: 取得するメッセージIDごとにパース処理を分けたい
class Parser
{
public:
    Parser(std::vector<CANID> lstMonitoringTarget);
    bool getCanData(uint8_t id, uint8_t length, uint32_t data);
private:
    std::vector<CANID> D_lstMonitoringTarget;
    bool extractDataFromCanFlame(CANID nCanID, uint32_t& data);
    bool extractRPM(uint32_t& data);
};

#endif /* PARSER_H */