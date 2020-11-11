#pragma once

#include <boost/asio.hpp>

class Worker;

class WorkerServer
{

public:
    WorkerServer(boost::asio::io_service& io_service_, short port_);

private:
    void Accept ();

    void Check_Accept (Worker* worker_, const boost::system::error_code& error_);

    boost::asio::io_service& m_io_service;
    boost::asio::ip::tcp::acceptor m_acceptor;

};