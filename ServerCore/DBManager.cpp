#include"pch.h"
#include "DBManager.h"


bool DBManager::Init(const DBConfig& cfg, int32 workerCount)
{
    // 중복 Init 방지
    bool expected = false;
    if (!s_inited.compare_exchange_strong(expected, true))
    {
        // 이미 Init 된 상태
        return true;
    }

    ASSERT_CRASH(workerCount > 0);

    s_pool = std::make_unique<DBConnectionPool>();
    s_queue = std::make_unique<DBJobQueue>();
    s_worker = std::make_unique<DBWorker>();

    // Pool Init 실패하면 전체 Init 실패 처리
    if (s_pool->Init(cfg) == false)
    {
        // Init 실패했으니 inited 되돌리고 자원 정리
        s_worker.reset();
        s_queue.reset();
        s_pool.reset();
        s_inited.store(false);
        return false;
    }

    s_worker->Start(s_queue.get(), workerCount);

    cout << "[DB] DBManager Init OK. workers=" << workerCount
        << " pool=" << cfg.PoolSize() << endl;

    return true;
}

void DBManager::Shutdown()
{
    if (s_inited.exchange(false) == false)
        return; // 이미 종료됨/초기화 안 됨

    if (s_worker)
        s_worker->Stop();     // 내부에서 queue shutdown 포함(너 구현에 따라)

    if (s_pool)
        s_pool->Shutdown();

    // 순서상 Worker/Queue/Pool 정리
    s_worker.reset();
    s_queue.reset();
    s_pool.reset();

    cout << "[DB] DBManager Shutdown OK." << endl;
}

DBConnectionPool& DBManager::Pool()
{
    EnsureInitialized();
    return *s_pool;
}

DBJobQueue& DBManager::Queue()
{
    EnsureInitialized();
    return *s_queue;
}

void DBManager::Enqueue(DBJobQueue::Job job)
{
    EnsureInitialized();
    s_queue->Push(std::move(job));
}

bool DBManager::IsInitialized()
{
    return s_inited.load();
}

void DBManager::EnsureInitialized()
{
    ASSERT_CRASH(s_inited.load() == true);
    ASSERT_CRASH(s_pool != nullptr);
    ASSERT_CRASH(s_queue != nullptr);
    ASSERT_CRASH(s_worker != nullptr);
}
