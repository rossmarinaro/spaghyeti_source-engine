
#include <iostream>
#include <fstream>
#include <ctime>

#include <chrono>
#include <iomanip>
#include <thread>

#include "../app/app.h"
#include "../renderer/renderer.h"

using namespace /* SPAGHYETI_CORE */ System;
using namespace std::chrono_literals;


Time::Time(float t): 
    m_lastTime(0.0f), 
    m_time(t)
{
}

void Time::Update(double t)
{

    Application::game->time->m_time = (float)t; //glQueryCounter

    Time delta = Application::game->time->m_time - Application::game->time->m_lastTime;
    
    Application::game->time->m_lastTime = Application::game->time->m_time;

    m_delta = delta;

    Game::UpdateFrame(); 
}



void Time::RunClock(int milliseconds)
{

    const auto timer_duration = std::chrono::minutes(milliseconds); //90mins

    const auto start = std::chrono::steady_clock::now();

    Application::game->time->time_left = timer_duration - (std::chrono::steady_clock::now() - start);

    while (Application::game->time->time_left > 0s)
    {

        const auto hrs = std::chrono::duration_cast<std::chrono::hours>(Application::game->time->time_left);
        const auto mins = std::chrono::duration_cast<std::chrono::minutes>(Application::game->time->time_left - hrs);
        const auto secs = std::chrono::duration_cast<std::chrono::seconds>(Application::game->time->time_left - hrs - mins);

        std::cout << std::setfill('0') << std::setw(2) << hrs.count() << ":"
                    << std::setfill('0') << std::setw(2) << mins.count() << ":"
                    << std::setfill('0') << std::setw(2) << secs.count() << "\n";

        std::this_thread::sleep_for(1s);

        Application::game->time->time_left = timer_duration - (std::chrono::steady_clock::now() - start);

    }
}


//--------------- delayed call (setTimeout)


void Time::delayedCall(int milliseconds, std::function<void()> &&fn_ptr)
{

    std::thread ([=] (){
        
        std::this_thread::sleep_for(std::chrono::milliseconds((milliseconds)));
    
        fn_ptr();

    }).detach();

}


//-------------- recursive interval timeout


void Time::setInterval(int milliseconds, std::function<void()> &&fn_ptr, std::mutex& m)
{

    if (exitFlag.load())
        return;

    std::thread ([=, &m]
    (){

        m.lock();

        while(!exitFlag.load())
        {
 
            if (exitFlag.load())
                return;

            std::this_thread::sleep_for(std::chrono::milliseconds((milliseconds)));
    
            if (exitFlag.load())
                return;

            fn_ptr();

        }

        m.unlock();

    }).detach();     

}

