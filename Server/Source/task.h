#ifndef _TASK_H_
#define _TASK_H_

#include "types.h"
#include <string>
#include <vector>
#include <boost/asio.hpp>

class Connection;

class Task
{
public:
    Task (std::string& destination_, std::string& operation_, Connection* connection_, bool GZiped_);
    ~Task ();

    void TaskTimeOut ();

    void CancelTask ();

    uint32 GetTaskID () const;

    void PrepareResponse (size_t responseLength_);
    void AppendData (char* data_, size_t length_);
    bool IsResponseComplete ();

    void SendResponse ();

    bool operator==(const Task& other_);
    bool operator==(const Task* other_);

private:

    volatile bool m_taskCompleted;
    uint32 m_taskID;
    Connection* m_connection;
    boost::asio::deadline_timer m_timeout;
    std::string m_taskResponse;
    uint32 m_taskResponseSize;
    bool m_isGZiped;
    static uint32 taskID;
};

class TaskSelector
{
public:
    TaskSelector (uint32 taskID_)
        :m_taskID(taskID_)
    {
    }

    inline bool operator () (const Task* first)
    {
        return (first->GetTaskID() == m_taskID);
    }

    inline bool operator () (const Task& first)
    {
        return (first.GetTaskID() == m_taskID);
    }

    inline bool operator == (const Task* first)
    {
        return (first->GetTaskID() == m_taskID);
    }

    inline bool operator == (const Task& first)
    {
        return (first.GetTaskID() == m_taskID);
    }
private:
    int m_taskID;
};

#endif
