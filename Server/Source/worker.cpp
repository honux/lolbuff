#include "worker.h"

#include <boost/bind.hpp>
#include "taskHolder.h"
#include "task.h"

#define MESSAGE_MAX_SIZE        1408-9
#define TASK_MESSAGE_CREATION   0x01

uint32 Worker::s_uidCounter = 1;

Worker::Worker (boost::asio::io_service& io_service_)
: m_socket(io_service_),
  m_taskID(0),
  m_uid(s_uidCounter++),
  m_isReceiving(false)
{
}

Worker::~Worker ()
{
    Workers::GetInstance().UnsubscribeWorker(m_uid);
    Workers::GetInstance().ReleaseCredentials(m_username, m_password);

    CloseConnection();
}

boost::asio::ip::tcp::socket& Worker::GetSocket ()
{
    return m_socket;
}

uint32 Worker::GetUniqueID ()
{
    return m_uid;
}

void Worker::CloseConnection ()
{
    boost::system::error_code error;
    m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
    m_socket.close(error);
}

void Worker::SendData (const char* data_, size_t dataLength_)
{
    // Sadly, it must be this way since Microsoft Windows® limit on 65536 data.
    size_t left = dataLength_;
    while (left > 0)
    {
        uint length = left;
        if (length > MESSAGE_MAX_SIZE)
        {
            length = MESSAGE_MAX_SIZE;
        }

        boost::asio::async_write(m_socket, boost::asio::buffer(data_, length), 
                boost::bind(&Worker::_HandleErrors, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

        data_ += length;
        left -= length;
    }
}

void Worker::AcceptWorker ()
{
    m_socket.async_read_some(boost::asio::buffer(m_bufferData, max_length), 
        boost::bind(&Worker::_CheckAccept, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void Worker::_CheckAccept (const boost::system::error_code& error_, size_t dataLength_)
{
    if (error_ || dataLength_ <= 17 || m_bufferData[0] != (char)0xFA || strncmp(&m_bufferData[1], "eXMAnHcDl ueTi0", 15) != 0)
    {
        printf("%s", error_.message().c_str());
        delete this;
        return;        
    }
    else
    {
        char buffer[200];
        std::pair<std::string, std::string> credentials = Workers::GetInstance().RequestCredentials();
        m_username = credentials.first;
        m_password = credentials.second;
        buffer[0] = m_username.length();
        sprintf(buffer+1, "%s", m_username.c_str());
        buffer[buffer[0]+1] = m_password.length();
        sprintf(buffer+buffer[0]+2, "%s", m_password.c_str());

        SendData(buffer, buffer[0]+buffer[buffer[0]+1]+2);

        m_socket.async_read_some(boost::asio::buffer(m_bufferData, max_length), 
        boost::bind(&Worker::_WaitConnection, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }
}

void Worker::_WaitConnection (const boost::system::error_code& error_, size_t dataLength_)
{
    if (error_ || m_bufferData[0] != (char)0xFF)
    {
        delete this;
        return;        
    }
    else
    {
        Workers::GetInstance().SubscribeWorker(this);
        m_socket.async_read_some(boost::asio::buffer(m_bufferData, max_length), 
        boost::bind(&Worker::_ReceiveData, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }
}

void Worker::_ReceiveData (const boost::system::error_code& error_, size_t dataLength_)
{
    if (error_)
    {
        delete this;
        return;
    }
    else if (m_isReceiving)
    {
        Task* task = TaskHolder::GetInstance().Find(m_taskID);

        if (task)
        {
            task->AppendData(m_bufferData, dataLength_);
            if (task->IsResponseComplete())
            {
                task->SendResponse();
                m_isReceiving = false;
            }
        }
        else
        {
            m_isReceiving = false;
        }
    }
    else if (m_bufferData[0] == TASK_MESSAGE_CREATION)
    {
        m_taskID = *(uint32*)&m_bufferData[1];
        Task* task = TaskHolder::GetInstance().Find(m_taskID);
        uint32 dataLength = *(uint32*)&m_bufferData[5];

        if (task)
        {
            task->PrepareResponse(dataLength);
            task->AppendData(&m_bufferData[9], dataLength_-9);
            
            if (task->IsResponseComplete())
            {
                task->SendResponse();
            }
            else
            {
                m_isReceiving = true;
            }
        }
    }
    // We are not receiving nor creating a packet, probably something went wrong. Ignore it for now.
    m_socket.async_read_some(boost::asio::buffer(m_bufferData, max_length), 
        boost::bind(&Worker::_ReceiveData, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void Worker::_HandleErrors (const boost::system::error_code& error_, size_t dataLength_)
{
    if (error_)
    {
        delete this;
    }
}