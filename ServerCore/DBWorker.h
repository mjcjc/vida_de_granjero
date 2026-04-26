#pragma once
#include "pch.h"

class DBJobQueue;

class DBWorker
{
public:
    void Start(DBJobQueue* queue, int32 workerCount);
    void Stop();

private:
    void WorkerLoop();

private:
    DBJobQueue* _queue = nullptr;
    Atomic<bool> _running = false;
    vector<thread> _threads;
};