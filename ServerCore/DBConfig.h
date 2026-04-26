#pragma once
#include"pch.h"


class DBConfig
{
public:
    DBConfig() = default;

    void SetHost(const string& host) { _host = host; }
    void SetUser(const string& user) { _user = user; }
    void SetPass(const string& pass) { _pass = pass; }
    void SetSchema(const string& schema) { _schema = schema; }
    void SetPoolSize(int32 size) { _poolSize = size; }

    const string& Host() const { return _host; }
    const string& User() const { return _user; }
    const string& Pass() const { return _pass; }
    const string& Schema() const { return _schema; }
    int32 PoolSize() const { return _poolSize; }

private:
    string _host = "tcp://127.0.0.1:3306";
    string _user = "game_user";
    string _pass = "StrongPassword!";
    string _schema = "game_db";
    int32  _poolSize = 16;
};