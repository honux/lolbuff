#ifdef _WIN32
    #include <winsock2.h>
    #include <Windows.h>
    static inline int close(int fd) { return closesocket(fd); }
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netdb.h>
    #include <unistd.h>
#endif

#include "client.h"
#include "bigEndian.h"
#include "requestTypes.h"
#include "callbacks.h"
#include "config.h"

#include <iostream>
#include <fstream>

#include <boost/array.hpp>
#include <boost/asio.hpp>

// config loading
#include "minini/minIni.h"

boost::asio::ip::tcp::socket* g_socket;
volatile bool g_testStatus = true;
volatile bool g_isConnected = false;

config g_config;

void UpdateClientVersion (const char* version_, const char* configFile_)
{
    if (ini_puts("LeagueOfLegends", "version", version_, configFile_) == 0)
    {
        /*
        std::fstream file;
        file.open(configFile, std::ios::out);
        file << "[general]\n";
        file << "serverAddress = " << g_config.serverAddr << "\n";
        file << "serverPort = " << g_config.serverPort << "\n";
        file << "\n";
        file << "[LeagueOfLegends]\n";
        file << "version = " << version_ << "\n";
        file << "loginServerAddress = " << g_config.leagueLoginServerAddress << "\n";
        file << "gameServerAddress = " << g_config.leaguegameServerAddress << "\n";
        file << "gameServerPort = " << g_config.leaguegameServerPort << "\n";
        file << "\n";
        file.close();
        */
    }
}

void CheckConnection (Client* client_)
{
    while (!boost::this_thread::interruption_requested())
    {
        printf("testing.\n");
        if (!client_->IsConnected() || !g_testStatus || !g_socket->is_open())
        {
            exit(EXIT_SUCCESS);
        }
        g_testStatus = false;
        client_->TestClient();
        boost::this_thread::sleep_for(boost::chrono::minutes(1));
    }
}

void IsConnected ()
{
    boost::this_thread::sleep_for(boost::chrono::minutes(5));
    if (!g_isConnected)
    {
        exit(EXIT_SUCCESS);
    }
}

int main (int argc_, char** argv_)
{
    if (argc_ < 2)
    {
        return 0;
    }
    const char* configFile = argv_[1];
    srand(time(NULL));
    puts("Loading configuration file....");

    if (ini_gets("general", "serverAddress", "", g_config.serverAddr, 256, configFile) == 0 ||
        ini_gets("general", "serverPort", "", g_config.serverPort, 256, configFile) == 0 ||
        ini_gets("LeagueOfLegends", "version", "", g_config.leagueVersion, 256, configFile) == 0 ||
        ini_gets("LeagueOfLegends", "loginServerAddress", "", g_config.leagueLoginServerAddress, 256, configFile) == 0 ||
        ini_gets("LeagueOfLegends", "gameServerAddress", "", g_config.leaguegameServerAddress, 256, configFile) == 0 ||
        ini_gets("LeagueOfLegends", "gameServerPort", "", g_config.leaguegameServerPort, 256, configFile) == 0
       )
    {
        puts("Failed to read the configuration file.");
        return 0;
    }

    puts("Configuration file loaded successfuly.");

    boost::asio::io_service io_service;
    boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);
    boost::asio::ip::tcp::resolver resolver(io_service);
    boost::asio::ip::tcp::resolver::query query(g_config.serverAddr, g_config.serverPort);
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    boost::asio::ip::tcp::socket socket(io_service);
    boost::system::error_code error;
    Client* client = nullptr;
    
    // You got 5 minutes to connect to the API server and the riot servers, GO!
    boost::thread(boost::bind(&IsConnected));

    boost::asio::connect(socket, endpoint_iterator, error);
    if (error)
    {
        puts("Failed to connect to the API server.");
        return 0;
    }
    
    g_socket = &socket;
    {
        char buffer[255];

        buffer[0] = (char)0xFA;
        memcpy(&buffer[1], "eXMAnHcDl ueTi0", 16);

        socket.write_some(boost::asio::buffer(buffer, 18), error);


        boost::array<char, 200> credentials;
        size_t len = socket.read_some(boost::asio::buffer(credentials), error);

        std::string username(&credentials[1], credentials[0]);
        std::string password(&credentials[credentials[0]+2], credentials[credentials[0]+1]);
        printf("got credentials for %s.\n", username.c_str());
        client = new Client(username.c_str(), password.c_str(), g_config.leagueVersion, io_service, ctx);
    }

    // It will never reach the limit, since it already took some time from the 5 minute limit
    for (int i = 0; i < 60*10*4; i++)
    {
        boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
        if (client->IsConnected() || client->GetError() != Client::ErrorCode::No_error)
        {
            break;
        }
    }
    
    if (!client->IsConnected() || client->GetError() != Client::ErrorCode::No_error)
    {
        puts("Failed to connect the client.");
        if (client->GetError() == Client::ErrorCode::Wrong_Client_Version)
        {
            UpdateClientVersion(client->GetErrorDescription().c_str(), configFile);
            return 0;
        }
        else
        {
            return 0;
        }
    }
    g_isConnected = true;
    {
        char connected = (char)0xFF;
        socket.write_some(boost::asio::buffer(&connected, 1), error);
    }
    boost::thread(boost::bind(&CheckConnection, client));

    for (;;)
    {
        boost::array<char, 512> buffer;
        uint8 requestType;
        uint32 requestID;
        size_t offset;
        const char* destination;
        const char* operation;
        size_t len = socket.read_some(boost::asio::buffer(buffer), error);

        if (error)
        {
            //int FOOL;
            //printf("%s", error.message().c_str());
            //scanf("%d", &FOOL);
            // just abort, it will be restarted soon anyway
            return 0;
        }

        requestType = buffer[0];
        requestID = *((uint32*)&buffer[1]);
        offset = 7+(uint)buffer[5];
        destination = &buffer[6];
        operation = &buffer[offset+1];
        offset += (uint)buffer[offset]+2;

        switch ((uchar)requestType)
        {
            case RequestType::Numeric_Request:
            {
                uint32 number = *(uint32*)&buffer[offset];
                client->RequestGeneric(destination, operation, number, requestID);
                //printf("Numeric Request: %s-%s(%d).\n", destination, operation, number);
            }
            break;

            case RequestType::String_Request:
            {
                char* string = &buffer[offset+1];
                buffer[offset+buffer[offset]+1] = '\0';
                client->RequestGeneric(destination, operation, string, requestID);
                //printf("String Request: %s-%s(%s).\n", destination, operation, string);
            }
            break;

            case RequestType::List_Request:
            {
                uchar listSize = buffer[offset++];
                ds::List<uint32> numbersList;
                for (int index = listSize-1; index >= 0; index--)
                {
                    numbersList.InsertFirst(*(uint32*)&buffer[offset]);
                    offset += 4;
                }
                client->RequestGeneric(destination, operation, numbersList, requestID);
            }
            break;

            case RequestType::Generic_Request:
            {
                uchar listSize = buffer[offset++];
                ds::List<RequestThing> thingsList;
                for (int index = listSize-1; index >= 0; index--)
                {
                    RequestThing thing;
                    thing.m_type = (RequestType)buffer[offset++];
                    if (thing.m_type == RequestType::String_Request)
                    {
                        thing.m_data = &buffer[offset+1];
                        offset += buffer[offset]+1;
                    }
                    else if (thing.m_type == RequestType::Numeric_Request)
                    {
                        uint32 temp = *(uint32*)&buffer[offset];
                        thing.m_data = (void*)(temp);
                        offset += 4;
                    }
                    else
                    {
                        continue;
                    }
                    thingsList.InsertLast(thing);
                }
                client->RequestGeneric(destination, operation, thingsList, requestID);
            }
            break;

            case RequestType::Kill:
            {
                return 0;
            }
            break;
        }
            
    }

    if (client->GetError() == Client::ErrorCode::Wrong_Client_Version)
    {
        strcpy(g_config.leagueVersion, client->GetErrorDescription().c_str());
    }
    delete client;

    return 0;
}
