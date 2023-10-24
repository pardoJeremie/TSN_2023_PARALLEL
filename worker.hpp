//
//  worker.hpp
//  TSN_2023_Parallel
//
//  Created by pardo jérémie on 14/10/2023.
//

#ifndef worker_hpp
#define worker_hpp

#include <stdio.h>
#include <string>
#include <thread>
#include <iomanip>

class WorkerPool;

class Worker
{
public:
    using task_t = std::function<void()>;
    Worker(WorkerPool& pool);
    // detruire m_thread (std::thread::join)
    ~ Worker();
    
    void printState();
private:
    // m_thread loop,
    // le thread est bloquant si pas de tache
    void loop();
    
    //std::string m_name;// identifiant/nom du Worker
    static uint_fast16_t m_counter;
    uint_fast16_t m_id;
    uint_fast32_t m_nb_task = 0;
    std::thread m_thread; // thread utiliser pour executer une tache
    WorkerPool& m_pool;
};

#endif /* worker_hpp */
