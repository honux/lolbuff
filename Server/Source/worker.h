#pragma once

#include <boost/asio.hpp>
#include "workers.h"
#include <string>

class Worker
{
public:
    Worker (boost::asio::io_service& io_service_);
    ~Worker ();

    boost::asio::ip::tcp::socket& GetSocket ();
    uint32 GetUniqueID ();
    void SendData (const char* data_, size_t dataLength_);
    void CloseConnection ();
    void AcceptWorker ();

private:
    void _CheckAccept (const boost::system::error_code& error_, size_t dataLength_);
    void _WaitConnection (const boost::system::error_code& error_, size_t dataLength_);
    void _ReceiveData (const boost::system::error_code& error_, size_t dataLength_);
    void _HandleErrors (const boost::system::error_code& error_, size_t dataLength_);

    enum { max_length = 65535 };
    bool m_isReceiving;
    uint32 m_taskID;
    uint32 m_uid;
    std::string m_username;
    std::string m_password;

    boost::asio::ip::tcp::socket m_socket;
    char m_bufferData[max_length];

    static uint32 s_uidCounter;
};