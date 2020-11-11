#ifndef _SERVER_H_
#define _SERVER_H_

#include <boost/asio.hpp>

using boost::asio::ip::tcp;
class Connection;

class APIServer
{
public:
    APIServer(boost::asio::io_service& io_service_, short port_);

private:
    void Accept ();

    void Check_Accept (Connection* connection_, const boost::system::error_code& error_);

    boost::asio::io_service& m_io_service;
    boost::asio::ip::tcp::acceptor m_acceptor;
};

#endif