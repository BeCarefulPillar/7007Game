#ifndef _CELL_SERVER_HPP
#define _CELL_SERVER_HPP

#include "INetEvent.hpp"
#include "Cell.hpp"
#include "CellSemaphore.hpp"

#include <map>
#include <vector>

class CellServer {
private:
    //��ʽ
    std::map<SOCKET, CellClient *> _client;
    //����
    std::vector<CellClient *> _clientBuff;
    std::mutex _mutex;
    std::thread* _pThread;
    INetEvent* _pNetEvent;
    CellTaskServer _taskServer;    
    fd_set _fdReadBak;
    SOCKET _maxSocket;
    time_t _oldTime = CellTime::GetNowInMillSec();
    CellSemaphore _sem;
    int _id;
    bool _clientChange;
    bool _isRun;

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
        _pThread = nullptr;
        _pNetEvent = nullptr;
        _clientChange = true;
        _isRun = true;
    }

    ~CellServer() {
        Close();
        delete _pThread;
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
        _clientBuff.push_back(pClient); //���߳���Ҫʹ�ã�����Ҫ����
    }

    void Start() {
        _pThread = new std::thread(std::mem_fn(&CellServer::OnRun), this); //��Ա����ת��������ʹ�ö���ָ������ý��а�
        _pThread->detach();
        _taskServer.Start();
    }

    //�ر�socket
    void Close() {
        if (_isRun) {
            _taskServer.Close();
            _isRun = false;
            _sem.Wait();
        }
    }

    bool OnRun() {
        while (_isRun) {
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
            FD_ZERO(&fdRead);

            if (_clientChange) {
                _clientChange = false;
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

            timeval t{0, 1};
            int ret = select(_maxSocket + 1, &fdRead, nullptr, nullptr, &t); //select ����ƿ�� ���ļ���ֻ��64
            if (ret < 0) {
                printf("select ���� \n");
                Close();
                return false;
            } else if (ret == 0) {
                continue;
            }

            ReadData(fdRead);
            CheckTime();
        }
        ClearClients();
        _sem.Wakeup();
        return true;
    }

    void CheckTime() {
        auto nowTime = CellTime::GetNowInMillSec();
        auto dt = nowTime - _oldTime;
        _oldTime = nowTime;
        for (auto iter = _client.begin(); iter != _client.end();) {
            //���ͼ��
            iter->second->CheckSend(dt);
            //�������
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

    void ReadData(fd_set& fdRead) {
#ifdef WIN32
        for (int i = 0; i < fdRead.fd_count; i++) {
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
                printf("error socket \n");
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

    //�������� ����ճ�� ��ְ�
    int RecvData(CellClient* pClient) {
        char* szRevc = pClient->MsgBuf() + pClient->GetLastPos();
        int nLen = recv(pClient->GetSocket(), szRevc, REVC_BUFF_SIZE - pClient->GetLastPos(), 0);
        if (nLen <= 0) {
            return -1;
        }
        //pClient->ResetDTHeart();
        _pNetEvent->OnNetRecv(pClient);
        //��Ϣ����������β�����
        pClient->SetLastPos(pClient->GetLastPos() + nLen);
        //�ж���Ϣ���ȴ�����Ϣͷ
        while (pClient->GetLastPos() >= sizeof(DataHeader)) {
            //��ǰ��Ϣ
            DataHeader *hd = (DataHeader*)pClient->MsgBuf();
            if (pClient->GetLastPos() >= hd->dataLen) {
                int msgLen = hd->dataLen;
                //������Ϣ
                OnNetMsg(pClient, hd);
                //����ǰ��
                memcpy(pClient->MsgBuf(), pClient->MsgBuf() + msgLen, pClient->GetLastPos() - msgLen);
                //��Ϣβ��ǰ��
                pClient->SetLastPos(pClient->GetLastPos() - msgLen);
            } else {
                break;
            }
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
