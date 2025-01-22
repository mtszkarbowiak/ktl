// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#include "Threading/JobScheduler.h"

JobScheduler::JobScheduler(const int32 workers)
    : _threads{ workers }
{
    for (int32 i = 0; i < workers; ++i)
    {
        JobScheduler* schedulerPtr = this;
        WorkerContext context{ i };

        _threads.Add(std::thread(
            [schedulerPtr, context]
            {
                schedulerPtr->WorkerLoop(context);
            }
        ));
    }
}

JobScheduler::~JobScheduler()
{
    _running = false;

    {
        std::unique_lock<std::mutex> lock{ _someJobs.mutex };
        _someJobs.signal.notify_all();
    }
    {
        std::unique_lock<std::mutex> lock{ _noMoreJobs.mutex };
        _noMoreJobs.signal.notify_all();
    }

    for (auto& thread : _threads)
        thread.join();
}

auto JobScheduler::Dispatch(JobOperation&& jobOperation) -> JobLabel
{
    JobLabel label;
    {
        std::unique_lock<std::mutex> lock{ _jobs.mutex };
        label = _jobs.nextLabel++;
        Job job{ MOVE(jobOperation), label };
        _jobs.queue.PushBack(job);
    }

    _someJobs.signal.notify_one();
    return label;
}

void JobScheduler::WorkerLoop(const WorkerContext& workerContext)
{
    while (_running)
    {
        Nullable<Job> job{};
        {
            std::unique_lock<std::mutex> lock{ _jobs.mutex };
            if (!_jobs.queue.IsEmpty()) 
            {
                job.Set(Job{ _jobs.queue.PeekFront() });
                _jobs.queue.PopFront();
            }
        }

        if (job.HasValue())
        {
            job.Value().operation.operator()();

            bool moreJobs = true;
            {
                std::unique_lock<std::mutex> lock{ _jobs.mutex };
                if (_jobs.queue.IsEmpty())
                    moreJobs = false;
            }
            if (!moreJobs)
                _noMoreJobs.signal.notify_one();
        }
        else
        {
            std::unique_lock<std::mutex> lock{ _someJobs.mutex };
            _someJobs.signal.wait(lock);
        }
    }
};

void JobScheduler::WaitAll()
{
    int32 runningJobs;

    {
        std::unique_lock<std::mutex> lock{ _jobs.mutex };
        runningJobs = _jobs.queue.Count();
    }

    while (runningJobs > 0) 
    {
        {
            std::unique_lock<std::mutex> lock{ _noMoreJobs.mutex };
            _noMoreJobs.signal.wait(lock);
        }
        {
            std::unique_lock<std::mutex> lock{ _jobs.mutex };
            runningJobs = _jobs.queue.Count();
        }
    }
}

void JobScheduler::Wait(const JobLabel label)
{
	bool jobRunning = true;
    do
    {
        {
			jobRunning = false; // Reset the flag.
            std::unique_lock<std::mutex> lock{ _jobs.mutex };
            for (auto jobCursor = _jobs.queue.Values(); jobCursor; ++jobCursor)
            {
                if (jobCursor->label == label)
                {
                    jobRunning = true;
                    break;
                }
            }
        }

        if (jobRunning)
        {
            std::unique_lock<std::mutex> lock{ _noMoreJobs.mutex };
            _noMoreJobs.signal.wait(lock);
        }
    }
    while (jobRunning);
}
