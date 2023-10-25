//
//  workerPool.cpp
//  TSN_2023_Parallel
//
//  Created by pardo jérémie on 14/10/2023.
//

#include "workerPool.hpp"
#include <iostream>
#include <iomanip>

// creer les Worker
WorkerPool::WorkerPool(uint_fast32_t nb_threads) {
    while (nb_threads--)
        m_workers.push_back(std::make_unique<Worker>(*this));
}

// detruire les Workers
// 1 - passer m_should_stop a true
// 2 - notifier aux threads le changement
WorkerPool::~WorkerPool() {
    std::unique_lock<std::mutex> lck(m_tasks_mutex);

    m_should_stop = true;
    
    m_task_cv.notify_all();
}

// attends la terminaison de toutes les taches,
// en cours et en attente dans m_tasks.
// L'appelant est bloque tant que (m_running_tasks != 0) || (m_tasks.empty() == false)
void WorkerPool::wait_all() {
    std::unique_lock<std::mutex> lck(m_tasks_mutex);
    
    while(m_running_tasks || !m_tasks.empty())
        m_task_cv.wait(lck);
}

void WorkerPool::printStates() {
    std::cout << "threads | tasks"<< std::endl;
    std::cout << "--------|--------"<< std::endl;
    for (auto& worker : m_workers) // Convertie en solution 3 à la compilation ?
        worker->printState();
}

// Appeler par Worker pour recuperer une tache dans m_tasks:
// 1 - attend qu'une tache soit disponible (bloquant si pas de tache dispo)
// 2 - incremente m_running_tasks
// 3 - retourne la tache au Worker et/ou boolean indiquant la sortie de la boucle Worker::loop
WorkerPool::worker_data_t WorkerPool::get_task() { // attention, un seul worker doit utiliser cette fonction à la fois
    std::unique_lock<std::mutex> lck(m_tasks_mutex);
    
    if(m_tasks.empty()) {
        if(!m_should_stop)
            m_task_cv.wait(lck);
        return m_should_stop;
    }
    
    worker_data_t m_task = m_tasks.front();
    m_tasks.pop();
    m_running_tasks ++;

    return m_task;
}

// Appeler par Worker pour indiquer la terminaison d'une tache:
// 1 - decrementer m_running_tasks
// 2 - notifier qu'une tache est terminee (pour wait_all)
void WorkerPool::finalize_task() {
    std::unique_lock<std::mutex> lck(m_tasks_mutex);
    
    m_running_tasks --;
    
    m_task_cv.notify_all();
}
