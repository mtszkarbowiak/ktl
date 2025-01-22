// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/mk-stl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#include <gtest/gtest.h>

#include "Threading/JobScheduler.h"

TEST(JobScheduler, Simple)
{
    JobScheduler scheduler{ 4 };

    const auto now = std::chrono::high_resolution_clock::now();

    scheduler.Dispatch([]()
    {
        std::cout << "Job 1 - Start\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Job 1 - End\n";
    });
    scheduler.Dispatch([]()
    {
        std::cout << "Job 2 - Start\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Job 2 - End\n";
    });
    scheduler.Dispatch([]()
    {
        std::cout << "Job 3 - Start\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Job 3 - End\n";
    });

    const auto then = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::seconds>(then - now);
    const float durationSeconds = duration.count();
    GTEST_ASSERT_LT(durationSeconds, 2);

    scheduler.WaitAll();
}
