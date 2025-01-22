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
    scheduler.Dispatch([]()
    {
        std::cout << "Job 1 - Start" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Job 1 - End" << std::endl;
    });
    scheduler.WaitAll();
}
