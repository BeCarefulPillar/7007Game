#ifndef _CELL_SERVER_HPP
#define _CELL_SERVER_HPP

#include "INetEvent.hpp"
#include "Cell.hpp"
#include <map>
#include <vector>

class CellSenMsg2ClienTask : public CellTask {
    CellClient* _pClient;
    DataHeader* _pHeader;
public:
    CellSenMsg2ClienTask(CellClient* pClient, DataHeader* pHd) {
        _pClient = pClient;
        _pHeader = pHd;
    }
    ~CellSenMsg2ClienTask() {

    }
    virtual void DoTask() {
        _pClient->SendData(_pHeader);
        delete _pHeader;
    }
private:
};

class CellServer {
private:
    SOCKET _sock;
    //��ʽ
    std::map<SOCKET, CellClient *> _client;
    //����
    std::vector<CellClient *> _clientBuff;
    std::mutex _mutex;
    std::thread* _pThread;
    INetEvent* _pNetEvent;
    CellTaskServer _taskServer;
public:
    CellServer(SOCKET socket = INVALID_SOCKET) {
        _sock = socket;
        _pThread = nullptr;
        _pNetEvent = nullptr;
        _clientChange = true;
    }

    ~CellServer() {
        Close();
        _sock = INVALID_SOCKET;
        delete _pThread;
    }
    void AddSendTask(CellClient* pClient, DataHeader* pHd) {
        CellSenMsg2ClienTask *task = new CellSenMsg2ClienTask(pClient, pHd);
        _taskServer.AddTask(task);
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
        if (INVALID_SOCKET == _sock) {
            return;
        }
#ifdef _WIN32
        for (auto c : _client) {
            closesocket(c.second->GetSocket());
            delete c.second;
        }
        //closesocket(_sock);
#else
        for (auto c : _client) {
            close(c.second->GetSocket());
            delete c.second;
        }
        //close(_sock);
#endif
        _client.clear();
        _sock = INVALID_SOCKET;
    }

    fd_set _fdReadBak;
    bool _clientChange;
    SOCKET _maxSocket;
    bool OnRun() {
        while (IsRun()) {
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


            int ret = select(_maxSocket + 1, &fdRead, nullptr, nullptr, nullptr); //select ����ƿ�� ���ļ���ֻ��64
            if (ret < 0) {
                printf("select ���� \n");
                Close();
                return false;
            } else if (ret == 0) {
                continue;
            }
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
                        _client.erase(iter->first);
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
        return true;
    }
    //�Ƿ�����
    bool IsRun() {
        return _sock != INVALID_SOCKET;
    }

    //�������� ����ճ�� ��ְ�
    int RecvData(CellClient* client) {
        char* szRevc = client->MsgBuf() + client->GetLastPos();
        int nLen = recv(client->GetSocket(), szRevc, REVC_BUFF_SIZE - client->GetLastPos(), 0);
        if (nLen <= 0) {
            return -1;
        }
        _pNetEvent->OnNetRecv(client);
        //��Ϣ����������β�����
        client->SetLastPos(client->GetLastPos() + nLen);
        //�ж���Ϣ���ȴ�����Ϣͷ
        while (client->GetLastPos() >= sizeof(DataHeader)) {
            //��ǰ��Ϣ
            DataHeader *hd = (DataHeader*)client->MsgBuf();
            if (client->GetLastPos() >= hd->dataLen) {
                int msgLen = hd->dataLen;
                //������Ϣ
                OnNetMsg(client, hd);
                //����ǰ��
                memcpy(client->MsgBuf(), client->MsgBuf() + msgLen, client->GetLastPos() - msgLen);
                //��Ϣβ��ǰ��
                client->SetLastPos(client->GetLastPos() - msgLen);
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
