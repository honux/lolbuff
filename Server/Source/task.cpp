#include "task.h"
#include "taskHolder.h"
#include "time.h"
#include "connection.h"
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

uint32 Task::taskID = 0;

#define TASK_TIMEOUT_MAX        1500

Task::Task (std::string& destination_, std::string& operation_, Connection* connection_, bool GZiped_)
    :m_taskID(taskID++),
    m_connection(connection_),
    m_taskCompleted(false),
    m_timeout(connection_->GetIOService(), boost::posix_time::milliseconds(TASK_TIMEOUT_MAX)),
    m_taskResponseSize(0),
    m_isGZiped(true) // Temporary will stay like this
{
    m_timeout.async_wait(boost::bind(&Task::TaskTimeOut, this));
    connection_->SetRelatedTask(this);
}

Task::~Task ()
{
}

 void Task::TaskTimeOut ()
{
    if (!m_taskCompleted)
    {
        const char service_unavailable[] = "HTTP/1.0 408 Request Timeout\r\n"
         "Content-Length: 40\r\n"
         "Content-Type: application/json\r\n"
         "Connection: close\r\n"
         "\r\n"
         "{\"success\":false, \"code\":408, \"data\":{}}\r\n";
        m_connection->SendAndRelease(service_unavailable, strlen(service_unavailable));
    }

    TaskHolder::GetInstance().FreeTask(this);
}

 void Task::CancelTask ()
{
    boost::system::error_code error;
    m_timeout.cancel(error);
}

uint32 Task::GetTaskID () const
{
    return m_taskID;

}

void Task::PrepareResponse (size_t responseLength_)
{
    m_taskResponse = "HTTP/1.1 200 OK\r\n";
    if (m_isGZiped)
    {
        m_taskResponse.append("Content-Encoding: gzip\r\n");        
    }
    m_taskResponse.append("Content-Length: ");
    m_taskResponse.append(boost::lexical_cast<std::string>(responseLength_));
    m_taskResponse.append("\r\nContent-Type: application/json; charset=UTF-8\r\n\r\n");
    m_taskResponseSize = responseLength_;
}

void Task::AppendData (char* data_, size_t length_)
{
    m_taskResponse.append(data_, length_);
    m_taskResponseSize -= length_;
    if (IsResponseComplete())
    {
        m_taskCompleted = true;
        m_taskResponse.append("\r\n");
    }
}

bool Task::IsResponseComplete ()
{
    return (m_taskResponseSize == 0);
}

void Task::SendResponse ()
{
    if (IsResponseComplete())
    {
        m_connection->SendAndRelease(m_taskResponse.c_str(), m_taskResponse.length());
    }
}

bool Task::operator==(const Task& other_)
{
    if (m_taskID == other_.m_taskID)
    {
        return true;
    }
    return false;
}

bool Task::operator==(const Task* other_)
{
    if (other_ && m_taskID == other_->m_taskID)
    {
        return true;
    }
    return false;
}