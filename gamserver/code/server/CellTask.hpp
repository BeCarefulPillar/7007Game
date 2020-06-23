#ifndef _CELL_TASK_hpp
#define _CELL_TASK_hpp

#include<thread>
#include<mutex>
#include<list>
#include<functional>

class CellTaskServer {
    typedef std::function<void()> CellTask;
private:
    //Êý¾Ý
    std::list<CellTask> _tasks;
    //»º³å
    std::list<CellTask> _tasksBuf;
    std::mutex _mutex;
    bool _isRun = false;
public:
    CellTaskServer() {}
    ~CellTaskServer() {}

    void AddTask(CellTask task) {
        std::lock_guard<std::mutex> lock(_mutex);
        _tasksBuf.push_back(task);
    }

    void Start() {
        _isRun = true;
        std::thread t(std::mem_fn(&CellTaskServer::OnRun), this);
        t.detach();
    }

    void Close() {
        _isRun = false;
    }
private:
    void OnRun() {
        while (_isRun) {
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
    }
};
#endif