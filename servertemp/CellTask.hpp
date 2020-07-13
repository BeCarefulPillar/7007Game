#ifndef _CELL_TASK_hpp
#define _CELL_TASK_hpp

#include<thread>
#include<mutex>
#include<list>
#include<functional>

#include "CellThread.hpp"
class CellTaskServer {
    typedef std::function<void()> CellTask;
private:
    //数据
    std::list<CellTask> _tasks;
    //缓冲
    std::list<CellTask> _tasksBuf;
    std::mutex _mutex;
    CellThread _thread;
    bool _isRun = false;
public:
    CellTaskServer() {}
    ~CellTaskServer() {}

    void AddTask(CellTask task) {
        std::lock_guard<std::mutex> lock(_mutex);
        _tasksBuf.push_back(task);
    }

    void Start() {
        _thread.Start(nullptr, [this](CellThread* pThread) {
            OnRun(pThread);
        });
    }

    void Close() {
        _thread.Close();
    }

private:
    void OnRun(CellThread* pThread) {
        while (pThread->IsRun()) {
            if (!_tasksBuf.empty()) {
                std::lock_guard<std::mutex> lock(_mutex);
                for (auto pTask : _tasksBuf) {
                    _tasks.push_back(pTask);
                }
                _tasksBuf.clear();
            }
            if (_tasks.empty()) {
                std::chrono::milliseconds t(1);
                std::this_thread::sleep_for(t);
                continue;
            }
            for (auto pTask : _tasks) {
                pTask();
            }
            _tasks.clear();
        }

        for (auto pTask : _tasksBuf) {
            pTask();
        }
    }
};
#endif