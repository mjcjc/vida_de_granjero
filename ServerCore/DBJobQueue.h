#pragma once
#include "pch.h"

class DBJobQueue
{
public:
    using Job = std::function<void()>;

    void Push(Job job);
    bool Pop(OUT Job& outJob);
    void Shutdown();

private:
    Mutex _mtx;
    CondVar _cv;
    queue<Job> _jobs;
    Atomic<bool> _running = true;
};