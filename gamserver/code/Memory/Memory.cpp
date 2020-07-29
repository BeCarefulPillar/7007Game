#include <iostream>

#include <thread>
#include <condition_variable>

class CellSemaphore {
private:
    //阻塞--条件变量
    std::condition_variable _cv;
    std::mutex _mutex;
    int _wait = 0;
    int _wakeup = 0;

public:
    //阻塞当前线程
    void Wait() {
        std::unique_lock<std::mutex> lock(_mutex);
        if (--_wait < 0) {
            _cv.wait(lock, [this]()->bool {
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

CellSemaphore sem;
void cmdThread() {
    int index = 0;
    while (true) {
        std::cout << index<<std::endl;
        index++;
        if (index > 1000) {
            sem.Wakeup();
            break;
        } else {

        }
    }
}

int main()
{
    std::thread cmd(cmdThread, 1);
    cmd.detach(); //和主线程分离
    
    sem.Wait();
    while (1) {
        std::cout << "Hello" << std::endl;
    }

    std::cout << "Hello World!\n";
    return 0;
}