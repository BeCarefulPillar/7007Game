#ifndef _MESSAGE_HEADER_HPP_
#define _MESSAGE_HEADER_HPP_
#include<cstdint>
enum CMD {
    CMD_ERROR = 0,
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
    CMD_NEW_CLIENT_JOIN,
    CMD_HEART,
    CMD_HEART_RESULT,
};

struct DataHeader {
    DataHeader() {
        cmd = CMD_ERROR;
        dataLen = sizeof(DataHeader);
    };
    uint16_t cmd;
    uint16_t dataLen;
};

//DataPackage
struct Login : public DataHeader {
    Login() {
        cmd = CMD_LOGIN;
        dataLen = sizeof(Login);
    };
    char account[32];
    char password[32];
    char data[28];
};

struct LoginResult : public DataHeader {
    LoginResult() {
        cmd = CMD_LOGIN_RESULT;
        dataLen = sizeof(LoginResult);
        result = 0;
    };
    int result;
    char data[88];
};

struct Logout : public DataHeader {
    Logout() {
        cmd = CMD_LOGOUT;
        dataLen = sizeof(Logout);
    };
    char account[32];
};

struct LogoutResult : public DataHeader {
    LogoutResult() {
        cmd = CMD_LOGOUT_RESULT;
        dataLen = sizeof(LogoutResult);
        result = 0;
    };
    int result;
};

struct NewClientJoin : public DataHeader {
    NewClientJoin() {
        cmd = CMD_NEW_CLIENT_JOIN;
        dataLen = sizeof(NewClientJoin);
    };
    int sock;
};

struct Heart : public DataHeader {
    Heart() {
        cmd = CMD_HEART;
        dataLen = sizeof(Heart);
    };
};

struct HeartResult : public DataHeader {
    HeartResult() {
        cmd = CMD_HEART_RESULT;
        dataLen = sizeof(HeartResult);
    };
};
#endif