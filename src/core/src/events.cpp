#include "../../../build/sdk/include/utils.h"
#include "../../../build/sdk/include/events.h"


Events::Events():
    isMobile(false), 
    isMultiThreaded(false) {
        LOG("Events: initialized.");
}

//----------------------------

Events::~Events() {
    LOG("Events: shut down.");
}


//----------------------------
  
//Constructor to creates a thread pool with given number of threads

Events::EventPool::EventPool(size_t thread_count)
{
    // Creating worker threads 
    for (size_t i = 0; i < thread_count; ++i) 
    { 
        m_threads.emplace_back([this] 
        { 
            while (true) 
            { 
                std::function<void()> task; 
                // The reason for putting the below code 
                // here is to unlock the queue before 
                // executing the task so that other 
                // threads can perform enqueue tasks 
                { 
                    // Locking the queue so that data 
                    // can be shared safely 
                    std::unique_lock<std::mutex> lock(m_queue_mutex); 

                    // Waiting until there is a task to 
                    // execute or the pool is stopped 
                    m_listener.wait(lock, [this] { return !m_tasks.empty() || !active; }); 

                    // exit the thread in case the pool 
                    // is stopped and there are no tasks 
                    if (!active && m_tasks.empty()) 
                        return;  

                    // Get the next task from the queue 
                    task = std::move(m_tasks.front()); 
                    m_tasks.pop();  
                } 

                task(); 
            } 
        }); 
    } 

    LOG("Events: thread pool started.");

}

//------------------------------------

Events::EventPool::~EventPool()  
{
    { 
        // Lock the queue to update the stop flag safely 
        std::unique_lock<std::mutex> lock(m_queue_mutex); 
        active = false; 
    } 

    // Notify all threads 
    m_listener.notify_all(); 

    // Joining all worker threads to ensure they have 
    // completed their tasks 
    for (auto& thread : m_threads) 
        thread.join();   
        
    m_threads.clear();     

    LOG("Events: thread pool stopped.");

}


//------------------------------------
  
// Enqueue task for execution by the thread pool 
void Events::EventPool::Enqueue(std::function<void()> task) 
{ 
    { 
        std::unique_lock<std::mutex> lock(m_queue_mutex); 
        m_tasks.emplace(std::move(task)); 
    }

    m_listener.notify_one(); 
} 

  
