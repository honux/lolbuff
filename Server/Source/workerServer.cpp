#include "workerServer.h"

#include "worker.h"
#include <boost/bind.hpp>

WorkerServer::WorkerServer(boost::asio::io_service& io_service_, short port_)
:m_io_service(io_service_),
    m_acceptor(io_service_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port_))
{
    Accept();
}

void WorkerServer::Accept ()
{
    Worker* worker = new Worker(m_io_service);

    m_acceptor.async_accept(worker->GetSocket(), boost::bind(&WorkerServer::Check_Accept, this, worker, boost::asio::placeholders::error));
}

void WorkerServer::Check_Accept (Worker* worker_, const boost::system::error_code& error_)
{
    if (!error_)
    {
        worker_->AcceptWorker();
    }
    else
    {
        delete worker_;
    }

    Accept();
}
