#ifndef _CELL_TIMESTAME_hpp
#define _CELL_TIMESTAME_hpp
#include <chrono>
class CellTimestame {
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> _begin;
public:
    CellTimestame() {
        Update();
    }

    ~CellTimestame() {

    }

    void Update() {
        _begin = std::chrono::high_resolution_clock::now();
    }
    //S
    double GetElapsedSecond() {
        return GetElapsedInMicroSec() * 0.000001;
    }
    //ms
    double GetElapsedInMilliSec() {
        return GetElapsedInMicroSec() * 0.001;
    }
    //us
    double GetElapsedInMicroSec() {
        return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - _begin).count();
    }

};

#endif // !_CELL_TIMESTAME_hpp
