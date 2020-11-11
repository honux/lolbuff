#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include <boost/asio.hpp>
#include "workers.h"

class Task;

class Connection
{
public:
    Connection (boost::asio::io_service& io_service_);
    ~Connection ();

    boost::asio::ip::tcp::socket& GetSocket ();
    
    void CloseConnection ();

    void MainLoop ();

    void SendData (const char* data_, size_t dataLength_);

    void SendAndRelease (const char* data_, size_t dataLength_);

    void SetRelatedTask (Task* task_);

    boost::asio::io_service& GetIOService ();

private:
    void _ReceiveData (const boost::system::error_code& error_, size_t dataLength_);
    void _HandleErrors (const boost::system::error_code& error_, size_t dataLength_);

    enum { max_length = 65535 };
    bool m_isSending;
    int32 m_sendDataLength;
    uint m_id;

    boost::asio::ip::tcp::socket m_socket;
    char m_bufferData[max_length];
    Task* m_relatedTask;
};

#endif