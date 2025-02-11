#include <fstream>
#include <ctime>

#include <chrono>
#include <iomanip>
#include <thread>

#include "../../../build/sdk/include/app.h"
#include "../../../build/sdk/include/renderer.h"

using namespace std::chrono_literals;


Time::Time(float time): now(time) {}

void Time::Update(double _time)
{

    Time* time = System::Application::game->time;
    time->now = (float)_time; //glQueryCounter
    Time delta = time->now - s_last;
    
    s_last = time->now;
    time->delta = delta;

    glfwPollEvents(); 

    System::Application::game->UpdateFrame(); 

}


//--------------- delayed call (setTimeout)


void Time::delayedCall(int milliseconds, std::function<void()>&& fn_ptr, int repeat) 
{
    TimedEvent data { milliseconds, repeat, std::chrono::steady_clock::now(), fn_ptr };

    auto event = std::make_shared<TimedEvent>(data);

    if (std::find(System::Application::game->time->timed_events.begin(), System::Application::game->time->timed_events.end(), event) == System::Application::game->time->timed_events.end())
        System::Application::game->time->timed_events.push_back(event);

}


//--------------- delayed call to thread pool (setTimeout)


void Time::delayedCallThread(int milliseconds, std::function<void()>&& fn_ptr, int repeat)
{

    if (System::Application::isMultiThreaded)
        System::Application::eventPool->Enqueue([=] { 

            int times = repeat;

            if (times == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));

                if (System::Application::eventPool->active.load())
                    fn_ptr();
            }

            else {

                while(System::Application::eventPool->active.load() && times != 0) 
                { 
                    if (System::Application::eventPool->active.load())
                        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));

                    if (System::Application::eventPool->active.load()) {

                        fn_ptr(); 
                        
                        if (times != -1)
                            times--;
                    }
                }
            }
        
        });
}


//-------------- chrono timer


// void Time::RunClock(int milliseconds)
// {

//     const auto timer_duration = std::chrono::minutes(milliseconds); //90mins

//     const auto start = std::chrono::steady_clock::now();

//     Application::game->time->time_left = timer_duration - (std::chrono::steady_clock::now() - start);

//     while (Application::game->time->time_left > 0s)
//     {

//         const auto hrs = std::chrono::duration_cast<std::chrono::hours>(Application::game->time->time_left);
//         const auto mins = std::chrono::duration_cast<std::chrono::minutes>(Application::game->time->time_left - hrs);
//         const auto secs = std::chrono::duration_cast<std::chrono::seconds>(Application::game->time->time_left - hrs - mins);

//         std::cout << std::setfill('0') << std::setw(2) << hrs.count() << ":"
//                     << std::setfill('0') << std::setw(2) << mins.count() << ":"
//                     << std::setfill('0') << std::setw(2) << secs.count() << "\n";

//         std::this_thread::sleep_for(1s);

//         Application::game->time->time_left = timer_duration - (std::chrono::steady_clock::now() - start);

//     }
// }
