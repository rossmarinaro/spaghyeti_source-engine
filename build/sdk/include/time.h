#pragma once

#include <functional>
#include <ctime>
#include <memory>
//time class
class Time {

    public:

        static inline const double timeStep = 1.0f / 1000.0f;

        Time(float time = 0.0f);

        float delta;

        //timed event structure
        struct TimedEvent { int delay, repeat; float time_initiated; std::function<void()> callback; bool active = true; };

        //container of timed events
        std::vector<std::shared_ptr<TimedEvent>> timed_events;

        operator float() const { return m_now; } 

        inline float GetSeconds() const { return m_now; }
        inline float GetMilliseconds() const { return m_now * 1000; }

        //use this method for single threaded callbacks
        static void DelayedCall(int milliseconds, std::function<void()>&& fn_ptr, int repeat = 0);  
 
        //use this method for multi threaded callbacks. no-op if multi threading disabled 
        static void DelayedCallThread(int milliseconds, std::function<void()>&& fn_ptr, int repeat = 0);

        static void Update(double time);
        //static void RunClock(int milliseconds);

    private:

        float m_now;

        static inline float s_last = 0.0f;
        //std::chrono::duration<double> time_left;
        
};
