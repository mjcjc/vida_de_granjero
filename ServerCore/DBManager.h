#pragma once
#include"pch.h"

#include "DBConfig.h"
#include "DBConnectionPool.h"
#include "DBJobQueue.h"
#include "DBWorker.h"

// DB 서브시스템 진입점(매니저 방식)
class DBManager
{
public:
    // 서버 시작 시 1회 호출
    static bool Init(const DBConfig& cfg, int32 workerCount);

    // 서버 종료 시 1회 호출
    static void Shutdown();

    // 사용처에서 접근할 API
    static DBConnectionPool& Pool();
    static DBJobQueue& Queue();

    // 편의 함수: DB 작업 하나를 큐에 넣기
    static void Enqueue(DBJobQueue::Job job);

    static bool IsInitialized();

private:
    static void EnsureInitialized();

private:
    // 전역처럼 보이지만 외부에 노출되지 않는 “단일 인스턴스”
    static inline std::unique_ptr<DBConnectionPool> s_pool;
    static inline std::unique_ptr<DBJobQueue>       s_queue;
    static inline std::unique_ptr<DBWorker>         s_worker;

    static inline Atomic<bool> s_inited = false;
};
