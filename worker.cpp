//
//  worker.cpp
//  TSN_2023_Parallel
//
//  Created by pardo jérémie on 14/10/2023.
//
#include <iostream>
#include <variant>

#include "worker.hpp"
#include "workerPool.hpp"

uint_fast16_t Worker::m_counter = 0;

Worker::Worker(WorkerPool& pool): m_pool( pool) {
    m_thread = std::thread(&Worker::loop,this);
    m_id = m_counter++;
}

// detruire m_thread (std::thread::join)
Worker::~Worker() {
    m_thread.join();
}

void Worker::printState() {
    std::cout << std::setw(6) << m_id << "  |"<< std::setw(6) << m_nb_task << std::endl;
}

// m_thread loop,
// le thread est bloquant si pas de tache
void Worker::loop() {
    while(true) {
        // data contient deux informations:
        // 1 - la task_t a executer (si presente)
        // 2 - est ce que m_thread doit sortir de sa boucle (break)
        auto data = m_pool.get_task();
        try {
            auto task = std::get<task_t>(data);
            task();
            m_nb_task++;
            m_pool.finalize_task();
        }
        catch(const std::bad_variant_access& /*ex*/) {// no tasks
            if(std::get<bool>(data)) // data return m_should_stop
                return;
        }
    }
}
