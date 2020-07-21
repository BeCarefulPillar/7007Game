#ifndef _CELL_SERVER_HPP
#define _CELL_SERVER_HPP

#include "INetEvent.hpp"
#include "Cell.hpp"
#include "CellSemaphore.hpp"

#include <map>
#include <vector>

class CellServer {
private:
    //正式
    std::map<SOCKET, CellClient *> _client;
    //缓冲
    std::vector<CellClient *> _clientBuff;
    std::mutex _mutex;
    INetEvent* _pNetEvent;
    CellTaskServer _taskServer;    
    fd_set _fdReadBak;
    SOCKET _maxSocket;
    time_t _oldTime = CellTime::GetNowInMillSec();
    CellThread _thread;
    int _id;
    bool _clientChange;
private:
    void ClearClients() {
        for (auto c : _client) {
            delete c.second;
        }
        _client.clear();
        for (auto c : _clientBuff) {
            delete c;
        }
        _clientBuff.clear();
    }
public:
    CellServer(int id) {
        _id = id;
        _pNetEvent = nullptr;
        _clientChange = true;
    }

    ~CellServer() {
        Close();
    }

    void AddSendTask(CellClient* pClient, DataHeader* pHd) {
        _taskServer.AddTask([pClient, pHd]() {
            pClient->SendData(pHd);
            delete pHd;
        });
    }

    void SetNetObj(INetEvent* event) {
        _pNetEvent = event;
    }

    int GetClientCount() {
        return _clientBuff.size() + _client.size();
    }

    void AddClient(CellClient* pClient) {
        std::lock_guard<std::mutex> lock(_mutex);
        _clientBuff.push_back(pClient); //在线程中要使用，所以要加锁
    }

    void Start() {
        _thread.Start(
            nullptr, 
            [this](CellThread* pThread) {
                OnRun(pThread);
            },
            [this](CellThread* pThread) {
                ClearClients();
            }
        );
        _taskServer.Start();
    }

    //关闭socket
    void Close() {
        _taskServer.Close();
        _thread.Close();
    }

    bool OnRun(CellThread* pThread) {
        while (pThread->IsRun()) {
            if (!_clientBuff.empty()) {
                std::lock_guard<std::mutex> lock(_mutex);
                for (auto pClient : _clientBuff) {
                    _client[pClient->GetSocket()] = pClient;
                }
                _clientBuff.clear();
                _clientChange = true;
            }

            if (_client.empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                _oldTime = CellTime::GetNowInMillSec();
                continue;
            }

            fd_set fdRead;
            fd_set fdWrite;
            //fd_set fdExc;
            
            if (_clientChange) {
                _clientChange = false;
                FD_ZERO(&fdRead);
                _maxSocket = _client.begin()->second->GetSocket();
                for (auto c : _client) {
                    FD_SET(c.first, &fdRead);
                    if (c.first > _maxSocket) {
                        _maxSocket = c.first;
                    }
                }
                memcpy(&_fdReadBak, &fdRead, sizeof(fd_set));
            } else {
                memcpy(&fdRead, &_fdReadBak, sizeof(fd_set));
            }
            //memcpy(&fdWrite, &_fdReadBak, sizeof(fd_set));
            //memcpy(&fdExc, &_fdReadBak, sizeof(fd_set));
            
            bool bNeedWrite = false;
            FD_ZERO(&fdWrite);
            for (auto iter: _client) {
                //检测可写客户端
                if (iter.second->NeedWrite()) {
                    FD_SET(iter.second->GetSocket(), &fdWrite);
                    bNeedWrite = true;
                }
            }

            timeval t{ 0, 1 }; 
            int ret = 0;
            //select 性能瓶颈 最大的集合只有64
            if (bNeedWrite) {
                ret = select(_maxSocket + 1, &fdRead, &fdWrite, nullptr, &t);
            } else {
                ret = select(_maxSocket + 1, &fdRead, nullptr, nullptr, &t);
            }
            if (ret < 0) {
                CellLog::Info("Cellserver %d.OnRun.select Error exit\n", _id);
                pThread->Exit();
                break;
            } 
//             else if (ret == 0) {
//                 continue;
//             }

            ReadData(fdRead);
            WriteData(fdWrite);
            //WriteData(fdExc);
            //CellLog::Info("CellServer%d.OnRun.select: fdRead = %d,fdWrite = %d \n", _id, fdRead.fd_count, fdWrite.fd_count);
//             if (fdExc.fd_count > 0){
//                 CellLog::Info("#### fdExc = %d\n", _id, fdExc.fd_count);
//             }
            CheckTime();
        }
        return true;
    }

    void CheckTime() {
        auto nowTime = CellTime::GetNowInMillSec();
        auto dt = nowTime - _oldTime;
        _oldTime = nowTime;
        for (auto iter = _client.begin(); iter != _client.end();) {
            //发送检测
            //iter->second->CheckSend(dt);
            //心跳检测
            if (iter->second->CheckHeart(dt)){
                if (_pNetEvent) {
                    _pNetEvent->OnNetLevel(iter->second);
                }
                delete iter->second;
                auto iterOld = iter;
                iter++;
                _client.erase(iterOld);
                _clientChange = true;
            } else {
                iter++;
            }

        }
    }

    void WriteData(fd_set& fdWrite) {
#ifdef WIN32
        for (int i = 0; i < (int)fdWrite.fd_count; i++) {
            auto iter = _client.find(fdWrite.fd_array[i]);
            if (iter != _client.end()) {
                int ret = iter->second->SendDataReal();
                if (ret == -1) {
                    if (_pNetEvent) {
                        _pNetEvent->OnNetLevel(iter->second);
                    }
                    _clientChange = true;
                    delete iter->second;
                    _client.erase(iter);
                }
            }
        }

#else
        std::vector <CellClient*> temp;
        for (auto c : _client) {
            if (FD_ISSET(c.first, &fdWrite) {
                int ret = iter->second->SendDataReal();
                if (ret == -1) {
                    if (_pNetEvent) {
                        _pNetEvent->OnNetLevel(c.second);
                    }
                    temp.push_back(c.second);
                    _clientChange = true;
                }
            }
        }

        for (auto client : temp) {
            _client.erase(client->GetSocket());
            delete client;
        }
#endif // WIN32

    }

    void ReadData(fd_set& fdRead) {
#ifdef WIN32
        for (int i = 0; i < (int)fdRead.fd_count; i++) {
            auto iter = _client.find(fdRead.fd_array[i]);
            if (iter != _client.end()) {
                int ret = RecvData(iter->second);
                if (ret == -1) {
                    if (_pNetEvent) {
                        _pNetEvent->OnNetLevel(iter->second);
                    }
                    delete iter->second;
                    _client.erase(iter);
                    _clientChange = true;
                }
            } else {
                CellLog::Info("error socket \n");
            }
        }

#else
        std::vector <CellClient*> temp;
        for (auto c : _client) {
            if (FD_ISSET(c.first, &fdRead)) {
                int ret = RecvData(c.second);
                if (ret == -1) {
                    if (_pNetEvent) {
                        _pNetEvent->OnNetLevel(c.second);
                    }
                    temp.push_back(c.second);
                    _clientChange = true;
                }
            }
        }

        for (auto client : temp) {
            _client.erase(client->GetSocket());
            delete client;
        }
#endif // WIN32

    }

    //接收数据 处理粘包 拆分包
    int RecvData(CellClient* pClient) {
        int nLen = pClient->RecvData();
        if (nLen <= 0) {
            return -1;
        }
        pClient->ResetDTHeart();
        //出发接受网络事件
        _pNetEvent->OnNetRecv(pClient);
        //判断完整消息
        while (pClient->HasMsg()) {
                //处理消息
                OnNetMsg(pClient, pClient->FrontMsg());
                //数据前移
                //移除消息队列（缓冲区）最前的一条数据
                pClient->PopFrontMsg();
        }

        return 0;
    }

    virtual void OnNetMsg(CellClient* pClient, DataHeader* pHd) {
        if (pClient && pHd) {
            _pNetEvent->OnNetMsg(this, pClient, pHd);
        }
    }
};


#endif // !_CELL_SERVER_HPP
