#ifndef _CELL_SEMAPHORE_HPP_
#define _CELL_SEMAPHORE_HPP_

#include <chrono>
#include <thread>

class CellSemaphore {
private:
    bool _isWaitExit = false;
public:
    CellSemaphore() {

    }
    ~CellSemaphore() {

    }

    void Wait() {
        _isWaitExit = true;
        while (_isWaitExit) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    void WakeUp() {
        _isWaitExit = false;
    }

private:

};


#endif // !_CELL_SEMAPHORE_HPP_
