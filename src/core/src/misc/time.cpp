
#include <iostream>
#include <fstream>
#include <ctime>

#include <chrono>
#include <iomanip>
#include <thread>

#include "../../../../build/sdk/include/app.h"
#include "../../../../build/sdk/include/renderer.h"

using namespace std::chrono_literals;


Time::Time(float t): m_now(t) {}

void Time::Update(double t)
{

    Time* time = System::Application::game->time;

    time->m_now = (float)t; //glQueryCounter

    Time delta = time->m_now - s_last;
    
    s_last = time->m_now;

    time->m_delta = delta;

    System::Game::UpdateFrame(); 
}



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


//--------------- delayed call (setTimeout)


void Time::delayedCall(int milliseconds, std::function<void()>&& fn_ptr)
{
 
    std::thread ([=] (){
        
        std::this_thread::sleep_for(std::chrono::milliseconds((milliseconds)));
    
        if (!System::Application::game->time->exitFlag)
            fn_ptr();

    }).detach();
 
}

//-------------- recursive interval timeout


void Time::setInterval(int milliseconds, std::function<void()>&& fn_ptr)
{

    std::thread ([=] (){

        if (System::Application::game->time->exitFlag.load())
            return;

        while(!System::Application::game->time->exitFlag.load()) {

            std::this_thread::sleep_for(std::chrono::milliseconds((milliseconds)));

            if (!System::Application::game->time->exitFlag)
                fn_ptr();
        }

        System::Application::game->time->exitFlag = false;

    }).detach();     

}


//-------------- recursive interval timeout with mutex


void Time::setInterval(int milliseconds, std::function<void()>&& fn_ptr, std::mutex& m)
{

    std::thread ([=, &m] () {

        if (System::Application::game->time->exitFlag.load())
            return;

        m.lock();

        while(!System::Application::game->time->exitFlag.load()) {

            std::this_thread::sleep_for(std::chrono::milliseconds((milliseconds)));
            fn_ptr();
        }

        m.unlock();

        System::Application::game->time->exitFlag = false;

    }).detach();     

}

