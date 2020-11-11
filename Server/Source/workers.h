#ifndef _WORKERS_H_
#define _WORKERS_H_

#include "types.h"
#include <string>
#include <vector>
#include <utility>
#include <list>

class Worker;

class Workers
{
public:

    Workers ();
    ~Workers ();

    void UnsubscribeWorker (uint32 position_);
    void SubscribeWorker (Worker* worker_);
    Worker* GetWorkerAtPosition (uint32 position_ );

    bool HasAvailableWorker ();

    Worker* GetAvaiableWorker ();

    std::string GetWorkersInformation ();

    std::pair<std::string, std::string> RequestCredentials ();
    void ReleaseCredentials (std::string username_, std::string password_);

    static Workers& GetInstance();

private:
    
    std::list<std::pair<std::string, std::string>> m_accountsList;
    uint32 m_lastWorker;
    std::vector<Worker*> m_workers;
};

#endif