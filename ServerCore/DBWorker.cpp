#include"pch.h"
#include "DBWorker.h"
#include "DBJobQueue.h"

void DBWorker::Start(DBJobQueue* queue, int32 workerCount)
{
    ASSERT_CRASH(queue != nullptr);
    ASSERT_CRASH(workerCount > 0);

    _queue = queue;
    _running.store(true);

    _threads.reserve(workerCount);

    for (int32 i = 0; i < workerCount; i++)
    {
        _threads.emplace_back([this]()
            {
                WorkerLoop();
            });
    }

    cout << "[DB] Worker Start. count=" << workerCount << endl;
}

void DBWorker::Stop()
{
    _running.store(false);

    if (_queue)
        _queue->Shutdown();

    for (thread& t : _threads)
    {
        if (t.joinable())
            t.join();
    }
    _threads.clear();

    cout << "[DB] Worker Stop." << endl;
}

void DBWorker::WorkerLoop()
{
    DBJobQueue::Job job;

    while (_running.load())
    {
        if (_queue->Pop(OUT job) == false)
            break;

        job();
    }
}