//
//  workerPool.hpp
//  TSN_2023_Parallel
//
//  Created by pardo jérémie on 14/10/2023.
//

#ifndef workerPool_hpp
#define workerPool_hpp

#include <stdio.h>
#include <thread>
#include <vector>
#include <queue>
#include <memory>
#include <utility>

#include "worker.hpp"

class WorkerPool
{
public:
    // retourne un task_t et/ou un boolean permettant de sortir de sa boucle
    // s'aider de std::optional, std::variant, std::any, std::tuple (au choix)
    using worker_data_t = std::variant<bool, std::function<void()>>;
    
    // permet au Worker d'acceder aux membres/methodes privees de WorkerPool
    friend class Worker;
    
    // creer les Worker
    WorkerPool(uint_fast32_t nb_threads);
    
    // detruire les Workers
    // 1 - passer m_should_stop a true
    // 2 - notifier aux threads le changement
    ~WorkerPool();
    
    // 1 - ajoute la tache a m_tasks
    // 2 - notifier la presence d'une nouvelle tache pour les Workers (s'aider de m_task_cv)
    // merci https://eli.thegreenplace.net/2014/perfect-forwarding-and-universal-references-in-c/ et https://stackoverflow.com/questions/7257144/when-to-use-stdforward-to-forward-arguments pour l'explication
    template<typename F, typename... A>
    void add_task(F&& task, A&&... args){
        std::unique_lock<std::mutex> lck(m_tasks_mutex);
        
        m_tasks.push([=](){task(args...);});
    }
    
    // attends la terminaison de toutes les taches,
    // en cours et en attente dans m_tasks.
    // L'appelant est bloque tant que (m_running_tasks != 0) || (m_tasks.empty() == false)
    void wait_all();
    
    void printStates();
    
private:
    // Appeler par Worker pour recuperer une tache dans m_tasks:
    // 1 - attend qu'une tache soit disponible (bloquant si pas de tache dispo)
    // 2 - incremente m_running_tasks
    // 3 - retourne la tache au Worker et/ou boolean indiquant la sortie de la boucle Worker::loop
    worker_data_t get_task();
    
    // Appeler par Worker pour indiquer la terminaison d'une tache:
    // 1 - decrementer m_running_tasks
    // 2 - notifier qu'une tache est terminee (pour wait_all)
    void finalize_task();
    std::queue<std::function<void()>> m_tasks;
    
    // mutex protege l'acces a m_tasks
    // et m_task_available_cv
    mutable std::mutex m_tasks_mutex;
    // utilise pour get_task
    std::condition_variable m_task_cv;
    std::vector<std::unique_ptr<Worker>> m_workers;
    
    // true si les threads doivent s'arreter (sortir de la boucle Worker::loop)
    bool m_should_stop = false;
    
    // nb tache en cours
    unsigned int m_running_tasks = 0;
};
#endif /* workerPool_hpp */
