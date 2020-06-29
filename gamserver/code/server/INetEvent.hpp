#ifndef _I_NET_EVENT_HPP
#define _I_NET_EVENT_HPP
//网络事件接口
#include "Cell.hpp"
#include "CellClient.hpp"
class CellServer;

class INetEvent {
public:
    virtual void OnNetJoin(CellClient* pClent) = 0;
    virtual void OnNetLevel(CellClient* pClent) = 0;
    virtual void OnNetMsg(CellServer* pCellServer, CellClient* pClent, DataHeader* pHd) = 0;
    virtual void OnNetRecv(CellClient* pClent) = 0;
};

#endif // !_I_NET_EVENT_HPP
