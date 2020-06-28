#ifndef _CELL_SEMAPHORE_HPP_
#define _CELL_SEMAPHORE_HPP_

#include <chrono>
#include <thread>
#include <condition_variable>

class CellSemaphore {
private:
    //����--��������
    std::condition_variable _cv;
    std::mutex _mutex;
    int _wait;
    int _wakeup;

public:
    //������ǰ�߳�
    void Wait() {
        std::unique_lock<std::mutex> lock(_mutex);
        if (--_wait < 0) {
            _cv.wait(lock, [this]()->bool{
                return _wakeup > 0;
            });
            --_wakeup;
        }
    }

    void Wakeup() {
        std::unique_lock<std::mutex> lock(_mutex);
        if (++_wait <= 0) {
            ++_wakeup;
            _cv.notify_one();
        }
    }
};


#endif // !_CELL_SEMAPHORE_HPP_
