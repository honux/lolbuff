#include "APIserver.h"
#include "connection.h"
#include <boost/bind.hpp>

APIServer::APIServer(boost::asio::io_service& io_service_, short port_)
:m_io_service(io_service_),
    m_acceptor(io_service_, tcp::endpoint(tcp::v4(), port_))
{
    Accept();
}

void APIServer::Accept ()
{
    Connection* connection = new Connection(m_io_service);

    m_acceptor.async_accept(connection->GetSocket(), boost::bind(&APIServer::Check_Accept, this, connection, boost::asio::placeholders::error));
}

void APIServer::Check_Accept (Connection* connection_, const boost::system::error_code& error_)
{
    if (!error_)
    {
        connection_->MainLoop();
    }
    else
    {
        delete connection_;
    }

    Accept();
}
