#ifndef _CELL_TASK_hpp
#define _CELL_TASK_hpp

#include<thread>
#include<mutex>
#include<list>

class CellTask {
public:
    CellTask() {

    }

    virtual ~CellTask() {

    }
    
    virtual void DoTask() {

    }
private:

};

typedef std::shared_ptr<CellTask> CellTaskPtr;
class CellTaskServer {
private:
    //数据
    std::list<CellTaskPtr> _tasks;
    //缓冲
    std::list<CellTaskPtr> _tasksBuf;

    std::mutex _mutex;
    bool _isRun = true;
public:
    CellTaskServer() {

    }
    ~CellTaskServer() {

    }

    void AddTask(CellTaskPtr& task) {
        std::lock_guard<std::mutex> lock(_mutex);
        _tasksBuf.push_back(task);
    }

    void Start() {
        std::thread t(std::mem_fn(&CellTaskServer::OnRun), this);
        t.detach();
    }

    void Stop() {
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
                pTask->DoTask();
            }
            _tasks.clear();
        }
    }
};


#endif