#ifndef _CELL_LOG_HPP_
#define _CELL_LOG_HPP_
#include "Cell.hpp"
class CellLog {
private:
    FILE* _logFile;
    CellTaskServer _taskServer;
public:
    CellLog() {
        _taskServer.Start();
    }
    ~CellLog() {
        _taskServer.Close();
        if (_logFile) {
            Info("CellLog:: fclose(logfile) \n");
            fclose(_logFile);
            _logFile = nullptr;
        }
    }
public:
    static CellLog& Instance() {
        static CellLog sLog;
        return sLog;
    }

    void SetLogPath(const char* logPath, const char* mode) {
        if (_logFile) {
            fclose(_logFile);
            _logFile = nullptr;
        }

        _logFile = fopen(logPath, mode);
        if (_logFile) {
            Info("CellLog::SetLogPath success, <%s,%s> \n", logPath, mode);
        } else {
            Info("CellLog::SetLogPath failed, <%s,%s> \n", logPath, mode);
        }
    }
    //Info
    //Debug
    //Warring
    //Error
    static void Info(const char* pStr) {
        CellLog* pLog = &Instance();
        if (pLog->_logFile) {
            fprintf(pLog->_logFile, "%s", pStr);
            fflush(pLog->_logFile);
        }
        printf("%s", pStr);
    }

    template<typename ... Args>
    static void Info(const char* pformat, Args ... args) {
        CellLog* pLog = &Instance();
        if (pLog->_logFile) {
            fprintf(pLog->_logFile, pformat, args...);
            fflush(pLog->_logFile);
        }
        printf(pformat, args...);
    }
};

#endif // !_CELL_LOG_HPP_
