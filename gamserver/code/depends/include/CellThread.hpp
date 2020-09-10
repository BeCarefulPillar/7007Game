#ifndef _CELL_THREAD_HPP_
#define _CELL_THREAD_HPP_

#include "CellSemaphore.hpp"

class CellThread {
private:
    typedef std::function<void(CellThread*)> EventCall;
private:
    EventCall _onCreate;
    EventCall _onRun;
    EventCall _onDestory;
    CellSemaphore _sem;
    //改变数据加锁
    std::mutex _mutex;
    bool _isRun = false;
public:
    static void Sleep(int time) {
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
    }
public:
    void Start(EventCall onCreate = nullptr, EventCall onRun = nullptr, EventCall onDestory = nullptr) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (!_isRun) {
            _isRun = true;

            if (onCreate) {
                _onCreate = onCreate;
            }
            if (onRun) {
                _onRun = onRun;
            }
            if (onDestory) {
                _onDestory = onDestory;
            }
            //线程
            std::thread t(std::mem_fn(&CellThread::OnWork), this);
            t.detach();
        }
    }
    //关闭线程
    void Close() {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_isRun) {
            _isRun = false;
            _sem.Wait();
        }
    }

    //工作函数中退出
    //不需要是要信号量阻塞等待
    void Exit() {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_isRun) {
            _isRun = false;
        }
    }

    bool IsRun() {
        return _isRun;
    }
protected:
    //线程运行时工作函数
    void OnWork() {
        if (_onCreate) {
            _onCreate(this);
        }
        if (_onRun) {
            _onRun(this);
        }
        if (_onDestory) {
            _onDestory(this);
        }
        _sem.Wakeup();
    }


};

#endif
