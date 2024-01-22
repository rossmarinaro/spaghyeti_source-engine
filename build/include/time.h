#pragma once
#include <functional>
#include <future>
#include <ctime>


//time class
class Time {

    public:

        static inline std::atomic<bool> exitFlag{false};

        static inline const double timeStep = 1.0f / 120.0f; //60

        static inline float m_delta;

        std::chrono::duration<double> time_left;

        Time(float t = 0.0f);

        operator float() const { return this->m_time; }

        inline float GetSeconds() const { return this->m_time; }
        
        inline float GetMilliseconds() const { return this->m_time * 1000; }

        static void delayedCall(int milliseconds, std::function<void()> &&fn_ptr);
        static void setInterval(int milliseconds, std::function<void()> &&fn_ptr, std::mutex& m);

        static void Update(double t);
        static void RunClock(int milliseconds);

    private:

        float m_time, m_lastTime;
        
};
