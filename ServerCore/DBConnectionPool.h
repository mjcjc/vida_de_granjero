#pragma once
#include"pch.h"

class DBConfig;

class DBConnectionPool
{
public:
    bool Init(const DBConfig& cfg);
    void Shutdown();

    // 커넥션을 빌림(없으면 대기). 서버 종료 중이면 nullptr 반환
    std::unique_ptr<sql::Connection> Acquire();

    // 커넥션 반납
    void Release(std::unique_ptr<sql::Connection> conn);

    int32 GetFreeCount() const;

private:
    mutable Mutex _mtx;
    CondVar _cv;
    queue<std::unique_ptr<sql::Connection>> _pool;
    Atomic<bool> _running = false;
};