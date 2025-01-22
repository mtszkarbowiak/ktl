// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Collections/Array.h"
#include "Collections/Dictionary.h"
#include "Collections/Ring.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

class JobScheduler
{
public:
    explicit JobScheduler(int32 workers);
    ~JobScheduler();

    using JobLabel     = int32;
    using JobOperation = std::function<void()>;

    auto Dispatch(JobOperation&& jobOperation) -> JobLabel;
    void WaitAll();
    void Wait(JobLabel label);

    JobScheduler(const JobScheduler&) = delete;
    JobScheduler(JobScheduler&&) = delete;

    auto operator=(const JobScheduler&) -> JobScheduler& = delete;
    auto operator=(JobScheduler&&) -> JobScheduler& = delete;


PRIVATE:
    struct Job
    {
        JobOperation operation;
        JobLabel     label;
    };

    Array<std::thread> _threads;
    std::atomic<bool>  _running{ true };

    struct
    {
        std::mutex mutex;
        Ring<Job>  queue;
        JobLabel   nextLabel;
    }
    _jobs;

    struct
    {
        std::condition_variable signal;
        std::mutex mutex;
    }
    _noMoreJobs;

    struct
    {
        std::condition_variable signal;
        std::mutex mutex;
    }
    _someJobs;

    struct WorkerContext
    {
        int32 workerIndex;
    };

    void WorkerLoop(const WorkerContext&);
};
