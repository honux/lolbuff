#include "connection.h"
#include "requestTypes.h"
#include "types.h"
#include "messageTypes.h"
#include "bigEndian.h"
#include "taskHolder.h"
#include "workers.h"
#include "request.h"
#include "task.h"

#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
uint id = 0;

Connection::Connection (boost::asio::io_service& io_service_)
: m_socket(io_service_),
  m_id(id++),
  m_relatedTask(nullptr),
  m_isSending(false),
  m_sendDataLength(0)
{
}

Connection::~Connection ()
{
    CloseConnection();
    if (m_relatedTask)
    {
        m_relatedTask->CancelTask();
    }
}

boost::asio::ip::tcp::socket& Connection::GetSocket ()
{
    return m_socket;
}

void Connection::CloseConnection ()
{
    boost::system::error_code error;
    m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
    m_socket.close(error);
}

void Connection::MainLoop ()
{
    m_socket.async_read_some(boost::asio::buffer(m_bufferData, max_length), 
        boost::bind(&Connection::_ReceiveData, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void Connection::SendData (const char* data_, size_t dataLength_)
{
    // Sadly, it must be this way since Microsoft Windows® limit on 65536 data.
    size_t left = dataLength_;
    while (left > 0)
    {
        if (left > 1408)
        {
            boost::asio::async_write(m_socket, boost::asio::buffer(data_, 1408), 
                boost::bind(&Connection::_HandleErrors, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
            data_ += 1408;
            left -= 1408;
        }
        else
        {
            boost::asio::async_write(m_socket, boost::asio::buffer(data_, left), 
                boost::bind(&Connection::_HandleErrors, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
            break;
        }
    }
}

void Connection::SendAndRelease (const char* data_, size_t dataLength_)
{
    m_isSending = true;
    m_sendDataLength = dataLength_;
    SendData(data_, dataLength_);
}

void Connection::SetRelatedTask (Task* task_)
{
    m_relatedTask = task_;
}

boost::asio::io_service& Connection::GetIOService ()
{
    return m_socket.get_io_service();
}

void Connection::_ReceiveData (const boost::system::error_code& error_, size_t dataLength_)
{
    if (error_)
    {
        delete this;
        return;
    }

    if (dataLength_ >= 4 && strncmp(&m_bufferData[0], "GET ", 4) == 0)
    {
        if (!Workers::GetInstance().HasAvailableWorker())
        {
            const char service_unavailable[] = "HTTP/1.1 503 Service Unavailable\r\n"
                "Content-Length: 40\r\n"
                "Content-Type: application/json\r\n"
                "Connection: close\r\n"
                "\r\n"
                "{\"success\":false, \"code\":503, \"data\":{}}\r\n";
            SendAndRelease(service_unavailable, strlen(service_unavailable));
        }
        else if (!Request::ParseRequest(&m_bufferData[4], dataLength_-4, this))
        {
            const char bad_request[] = "HTTP/1.1 400 Bad Request\r\n"
                "Content-Length: 40\r\n"
                "Content-Type: application/json\r\n"
                "Connection: close\r\n"
                "\r\n"
                "{\"success\":false, \"code\":400, \"data\":{}}\r\n";
            SendAndRelease(bad_request, strlen(bad_request));
        }
    }
    else
    {
        const char service_unavailable[] = "HTTP/1.1 503 Service Unavailable\r\n"
                "Content-Length: 40\r\n"
                "Content-Type: application/json\r\n"
                "Connection: close\r\n"
                "\r\n"
                "{\"success\":false, \"code\":503, \"data\":{}}\r\n";
        SendAndRelease(service_unavailable, strlen(service_unavailable));
    }
}

void Connection::_HandleErrors (const boost::system::error_code& error_, size_t dataLength_)
{
    if (error_)
    {
        if (m_isSending && m_sendDataLength > 0)
        {
            m_sendDataLength -= dataLength_;
            if (m_sendDataLength == 0)
            {
                delete this;
            }
        }
        else if (!m_isSending)
        {
            delete this;
        }
    }
    else
    {
        if (m_isSending && m_sendDataLength > 0)
        {
            m_sendDataLength -= dataLength_;
            if (m_sendDataLength == 0)
            {
                delete this;
            }
        }
    }
}