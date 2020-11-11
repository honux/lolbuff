#include "APIserver.h"
#include "workerServer.h"

#define API_ENDPOINT                    9876
#define WORKERS_ENDPOINT                1331

int main(int argc, char **argv)
{
    try
    {
        boost::asio::io_service io_service;

        WorkerServer ws(io_service, WORKERS_ENDPOINT);

        APIServer s(io_service, API_ENDPOINT);

        io_service.run();
    }
    catch(std::exception const& e)
    {
        printf("%s", e.what());
    }
    catch (...)
    {
        puts("wtf");
    }
    return 0;
}