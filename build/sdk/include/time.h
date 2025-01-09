#pragma once

#include <functional>
#include <ctime>

//time class
class Time {

    public:

        static inline const double timeStep = 1.0f / 1000.0f;

        Time(float t = 0.0f);

        float m_delta;

        std::chrono::duration<double> time_left;

        operator float() const { return this->m_now; }

        inline float GetSeconds() const { return this->m_now; }
        inline float GetMilliseconds() const { return this->m_now * 1000; }
        
        static void delayedCall(int milliseconds, std::function<void()>&& fn_ptr);
        static void setInterval(int milliseconds, std::function<void()>&& fn_ptr, int timesRemaining = -1);

        static void Update(double t);
        //static void RunClock(int milliseconds);

    private:

        float m_now;
        static inline float s_last = 0.0f;
        
};
