#include <condition_variable> 
#include <functional> 
#include <mutex> 
#include <queue> 
#include <thread> 
#include <atomic> 


class Events {

    public: 

        bool isMobile, isMultiThreaded;

        struct EventPool { 
            
            public: 

                EventPool(size_t count = std::thread::hardware_concurrency());
                ~EventPool();

                // Enqueue task for execution by the thread pool 
                void Enqueue(std::function<void()> task);

                // Flag to indicate whether the thread pool should stop 
                // or not 
                std::atomic_bool active { true };

            private: 

                // Vector to store worker threads 
                std::vector<std::thread> m_threads; 
            
                // Queue of tasks 
                std::queue<std::function<void()>> m_tasks; 
            
                // Mutex to synchronize access to shared data 
                std::mutex m_queue_mutex; 
            
                // Condition variable to signal changes in the state of 
                // the tasks queue 
                std::condition_variable m_listener;  
        }; 

        EventPool* pool;

        Events();
        ~Events();
};
  
