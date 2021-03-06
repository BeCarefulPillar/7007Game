﻿#ifndef _MESSAGE_HEADER_HPP_
#define _MESSAGE_HEADER_HPP_

enum CMD {
    CMD_ERROR = 0,
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
    CMD_NEW_CLIENT_JOIN,
};

struct DataHeader {
    DataHeader() {
        cmd = CMD_ERROR;
        dataLen = sizeof(DataHeader);
    };
    int cmd;
    int dataLen;
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
#endif