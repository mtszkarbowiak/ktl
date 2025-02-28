// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/mk-stl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#include <chrono>
#include <gtest/gtest.h>

#include "Threading/JobScheduler.h"

template<int Index, int DurationMs = 1000>
struct TestJob
{
    void operator()()
    {
        const std::string startInfo = "Job " + std::to_string(Index) + " - Start\n";
        const std::string endInfo = "Job " + std::to_string(Index) + " - End\n";

        std::cout << startInfo;
        std::this_thread::sleep_for(std::chrono::milliseconds(DurationMs));
        std::cout << endInfo;
    }
};

TEST(JobScheduler, WaitAllScheduled)
{
    JobScheduler scheduler{ 4 };

    const auto now = std::chrono::high_resolution_clock::now();

    scheduler.Dispatch(TestJob<1>{});
    scheduler.Dispatch(TestJob<2>{});
    scheduler.Dispatch(TestJob<3>{});

    scheduler.WaitAll();

    const auto then = std::chrono::high_resolution_clock::now();
    const auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(then - now).count();
    GTEST_ASSERT_LT(durationMs, 1100);
}

TEST(JobScheduler, WaitScheduled)
{
    JobScheduler scheduler{ 4 };
    const auto now = std::chrono::high_resolution_clock::now();

    const JobScheduler::JobLabel label1 = scheduler.Dispatch(TestJob<1>{});
    const JobScheduler::JobLabel label2 = scheduler.Dispatch(TestJob<2>{});
    const JobScheduler::JobLabel label3 = scheduler.Dispatch(TestJob<3>{});

    scheduler.Wait(label1);
    scheduler.Wait(label2);
    scheduler.Wait(label3);

    const auto then = std::chrono::high_resolution_clock::now();
    const auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(then - now).count();
    GTEST_ASSERT_LT(durationMs, 1100);
}

//TEST(JobScheduler, Joining)
//{
//    for (int i = 0; i < 1000; ++i)
//    {
//        JobScheduler scheduler{ 4 };
//
//        const JobScheduler::JobLabel label1 = scheduler.Dispatch(TestJob<1, 100>{});
//        const JobScheduler::JobLabel label2 = scheduler.Dispatch(TestJob<2, 100>{});
//        const JobScheduler::JobLabel label3 = scheduler.Dispatch(TestJob<3, 100>{});
//
//        scheduler.Wait(label1);
//        scheduler.Wait(label2);
//        scheduler.Wait(label3);
//    }
//}
