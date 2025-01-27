#pragma once

#include <functional>
#include <ctime>
#include <memory>
//time class
class Time {

    public:

        static inline const double timeStep = 1.0f / 1000.0f;

        Time(float t = 0.0f);

        float now, delta;

        std::chrono::duration<double> time_left;

        //container of timed events

        struct TimedEvent {
            int delay = 0;
            int repeat = 0;
            std::chrono::steady_clock::time_point time_initiated = std::chrono::steady_clock::now();
            std::function<void()> callback = []{};
        };

        std::vector<std::shared_ptr<TimedEvent>> timed_events;

        operator float() const { return now; }

        inline float GetSeconds() const { return now; }
        inline float GetMilliseconds() const { return now * 1000; }

        static void delayedCall(int milliseconds, std::function<void()>&& fn_ptr, int repeat = 0);   
        static void delayedCallThread(int milliseconds, std::function<void()>&& fn_ptr, int repeat = 0);

        static void Update(double t);
        //static void RunClock(int milliseconds);

    private:

        static inline float s_last = 0.0f;
        
};
