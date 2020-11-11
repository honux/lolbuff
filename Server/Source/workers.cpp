#include "workers.h"
#include "worker.h"

Workers::Workers ()
    :m_lastWorker(0)
{
    m_accountsList.push_back(std::pair<std::string, std::string>("ACCOUNT_NAME", "ACCOUNT_PASSWORD"));
}

Workers::~Workers ()
{
    while (m_workers.size() > 0)
    {
        m_workers.erase(m_workers.begin());
    }
}

void Workers::UnsubscribeWorker (uint32 uid_)
{
    for (std::vector<Worker*>::iterator it = m_workers.begin(); it != m_workers.end(); it++)
    {
        if (uid_ == (*it)->GetUniqueID())
        {
            m_workers.erase(it);
            if (m_lastWorker == m_workers.size())
            {
                m_lastWorker = 0;
            }
            return;
        }
    }
}

void Workers::SubscribeWorker (Worker* worker_)
{
    m_workers.push_back(worker_);
}

Worker* Workers::GetWorkerAtPosition (uint32 position_ )
{
    if (position_ < m_workers.size())
    {
        return m_workers[position_];
    }

    return NULL;
}

bool Workers::HasAvailableWorker ()
{
    return (m_workers.size() != 0);
}

Worker* Workers::GetAvaiableWorker ()
{
    if (++m_lastWorker == m_workers.size())
    {
        m_lastWorker = 0;
    }
    
    return m_workers[m_lastWorker];
}

std::string Workers::GetWorkersInformation ()
{
    std::string info("{\"code\":200,\"workers\":[");
    for (size_t i = 0; i < m_workers.size(); i++)
    {
        char infoStr[512];
        sprintf(infoStr, "{\"uid\":%d, \"address\":\"%s\"}", i, m_workers[i]->GetSocket().remote_endpoint().address().to_string().c_str());
        if (i != 0)
        {
            info.append(",");
        }
        info.append(infoStr);
    }
    info.append("]}");
    return info;
}

std::pair<std::string, std::string> Workers::RequestCredentials ()
{
    std::list<std::pair<std::string, std::string>>::iterator it = m_accountsList.begin();
    std::pair<std::string, std::string> value = *it;
    m_accountsList.erase(it);
    return value;
}

void Workers::ReleaseCredentials (std::string username_, std::string password_)
{
    m_accountsList.push_front(std::pair<std::string, std::string>(username_, password_));
}

Workers& Workers::GetInstance ()
{
    static Workers instance;
    return instance;
}
