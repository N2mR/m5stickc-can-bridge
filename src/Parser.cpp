#include "Parser.h"

Parser::Parser(std::vector<CANID> lstMonitoringTarget)
    :D_lstMonitoringTarget(lstMonitoringTarget)
{
}

bool Parser::getCanData(uint8_t id, uint8_t length, uint32_t data/*, 取得したデータをアウトプット引数で返却 */)
{
    // CANID抽出処理
    uint16_t nCanID = 0;
    for (size_t i = 0; i < D_lstMonitoringTarget.size(); i++)
    {  
        if (nCanID == D_lstMonitoringTarget[i])
        {

        }
    }

    return true;
}

bool Parser::extractDataFromCanFlame(CANID nCanID, uint32_t& data)
{
    switch(nCanID)
    {
        case CANID::RPM:
            extractRPM(data);
            break;
        default:
            break;
    }
    
    return true;
}