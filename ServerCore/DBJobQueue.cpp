#include "pch.h"
#include "DBJobQueue.h"

void DBJobQueue::Push(Job job)
{
    {
        LockGuard lock(_mtx);
        if (_running.load() == false)
            return;

        _jobs.push(std::move(job));
    }
    _cv.notify_one();
}

bool DBJobQueue::Pop(OUT Job& outJob)
{
    UniqueLock lock(_mtx);

    _cv.wait(lock, [&]()
        {
            return (_running.load() == false) || (_jobs.empty() == false);
        });

    if (_running.load() == false && _jobs.empty())
        return false;

    outJob = std::move(_jobs.front());
    _jobs.pop();
    return true;
}

void DBJobQueue::Shutdown()
{
    {
        LockGuard lock(_mtx);
        _running.store(false);
    }
    _cv.notify_all();
}