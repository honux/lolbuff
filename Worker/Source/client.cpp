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
#include "map.h"
#include "amf0.h"
#include "amf3.h"
#include "outTypedObject.h"
#include "bigEndian.h"
#include "rapidjson/document.h"
#include "base64.h"
#include "array.h"
#include "classDefinition.h"
#include "requestTypes.h"
#include "callbacks.h"
#include "message.h"
#include "config.h"
#include <iostream>
#include <sstream>

#define BUFFER_LENGTH 65535

#include "types.h"
#include "minini/minIni.h"

extern volatile bool g_testStatus;
extern config g_config;

void ReadData (size_t& bufferLeft_, utils::MemoryStream* stream_, SSL_Socket& socket_, uint8* to_, size_t size_);

Client::Client (const char* username_, const char* password_, const char* clientVersion_, boost::asio::io_service& io_service_, boost::asio::ssl::context& ctx_)
    :m_username(username_), m_password(password_), m_clientVersion(clientVersion_), m_socket(655350, g_config.leaguegameServerAddress, g_config.leaguegameServerPort, io_service_, ctx_), 
     m_DSID(NULL), m_authToken(NULL), m_currentIpAddress(NULL), m_sessionToken(NULL), m_callback(10), m_isConnected(false), m_invokeUID(2),
     m_errorCode(ErrorCode::No_error), m_testID(0)
{
    m_username = new char[strlen(username_)+1];
    strcpy((char*)m_username, username_);

    m_password = new char[strlen(password_)+1];
    strcpy((char*)m_password, password_);

    bool success = _DoRMTPSHandshake();
    if (!success)
    {
        SetError(ErrorCode::Failed_To_Do_HandShake);
    }

    m_readerThread = boost::thread(boost::bind(&Client::_ParsePacket, this));

    _doConnect();
}

Client::~Client ()
{
    m_heartBeatThread.interrupt();
    m_readerThread.interrupt();
    boost::this_thread::sleep_for(boost::chrono::seconds(5));

    delete[] m_DSID;
    delete[] m_username;
    delete[] m_password;
    delete[] m_authToken;
    delete[] m_sessionToken;
    delete[] m_currentIpAddress;
}

void Client::RequestGeneric (const char* destination_, const char* operation_, const char* string_, uint32 taskID_)
{
    if (!m_isConnected)
    {
        return;
    }

    utils::MemoryStream* outStream = m_socket.GetOutStream();
    OutTypedObject obj;
    char randomUID[37];
    ds::Array<AMF3_FUNCTION> arr(1);

    Client::GenerateRandomUID(randomUID);

    arr.SetElement(0, AMF3_WRITE_STRING_WITH_MARKER(outStream, string_));
    _WrapBody(&obj, destination_, operation_, randomUID, AMF3_WRITE_ARRAY_WITH_MARKER(outStream, &arr));

    m_callback.Insert(_Invoke(&obj), taskID_);

    m_socket.Send();
}

void Client::RequestGeneric (const char* destination_, const char* operation_, int value_, uint32 taskID_)
{
    if (!m_isConnected)
    {
        return;
    }

    utils::MemoryStream* outStream = m_socket.GetOutStream();
    OutTypedObject obj;
    char randomUID[37];
    ds::Array<AMF3_FUNCTION> arr(1);

    Client::GenerateRandomUID(randomUID);

    arr.SetElement(0, AMF3_WRITE_INTEGER_WITH_MARKER(outStream, value_));
    _WrapBody(&obj, destination_, operation_, randomUID, AMF3_WRITE_ARRAY_WITH_MARKER(outStream, &arr));

    m_callback.Insert(_Invoke(&obj), taskID_);

    m_socket.Send();
}

void Client::RequestGeneric (const char* destination_, const char* operation_, ds::List<uint32>& numberList_, uint32 taskID_)
{
    if (!m_isConnected)
    {
        return;
    }

    utils::MemoryStream* outStream = m_socket.GetOutStream();
    OutTypedObject obj;
    char randomUID[37];
    const ds::List<uint>& summonersList = numberList_;
    ds::Array<AMF3_FUNCTION> baseArray(1);
    ds::Array<AMF3_FUNCTION> numbersArray(summonersList.GetSize());
    Client::GenerateRandomUID(randomUID);
    int index = 0;

    for (ds::List<uint32>::ConstIterator it = summonersList.GetStart(); !summonersList.IsEnd(it); summonersList.Next(&it))
    {
        numbersArray.SetElement(index++, AMF3_WRITE_INTEGER_WITH_MARKER(outStream, *it));
    }

    baseArray.SetElement(0, AMF3_WRITE_ARRAY_WITH_MARKER(outStream, &numbersArray));
    _WrapBody(&obj, destination_, operation_, randomUID, AMF3_WRITE_ARRAY_WITH_MARKER(outStream, &baseArray));

    m_callback.Insert(_Invoke(&obj), taskID_);

    m_socket.Send();
}

void Client::RequestGeneric (const char* destination_, const char* operation_, ds::List<RequestThing>& thingList_, uint32 taskID_)
{
    if (!m_isConnected)
    {
        return;
    }

    utils::MemoryStream* outStream = m_socket.GetOutStream();
    OutTypedObject obj;
    char randomUID[37];
    const ds::List<RequestThing>& thingList = thingList_;
    ds::Array<AMF3_FUNCTION> thingsArray(thingList.GetSize());
    Client::GenerateRandomUID(randomUID);
    int index = 0;

    for (ds::List<RequestThing>::ConstIterator it = thingList.GetStart(); !thingList.IsEnd(it); thingList.Next(&it))
    {
        if ((*it).m_type == RequestType::String_Request)
        {
            thingsArray.SetElement(index++, AMF3_WRITE_STRING_WITH_MARKER(outStream, (*it).m_data));
        }
        else
        {
            thingsArray.SetElement(index++, AMF3_WRITE_INTEGER_WITH_MARKER(outStream, (*it).m_data));
        }
    }

    _WrapBody(&obj, destination_, operation_, randomUID, AMF3_WRITE_ARRAY_WITH_MARKER(outStream, &thingsArray));

    m_callback.Insert(_Invoke(&obj), taskID_);

    m_socket.Send();
}

bool Client::DoBeatHeart (uint beatCount_, char* timeString_)
{
    if (!m_isConnected)
    {
        return false;
    }

    utils::MemoryStream* outStream = m_socket.GetOutStream();
    OutTypedObject obj;
    char randomUID[37];
    ds::Array<AMF3_FUNCTION> arr(4);

    Client::GenerateRandomUID(randomUID);

    arr.SetElement(0, AMF3_WRITE_INTEGER_WITH_MARKER(outStream, m_accountId));
    arr.SetElement(1, AMF3_WRITE_STRING_WITH_MARKER(outStream, m_sessionToken));
    arr.SetElement(2, AMF3_WRITE_INTEGER_WITH_MARKER(outStream, beatCount_));
    arr.SetElement(3, AMF3_WRITE_STRING_WITH_MARKER(outStream, timeString_));

    _WrapBody(&obj, "loginService", "performLCDSHeartBeat", randomUID, AMF3_WRITE_ARRAY_WITH_MARKER(outStream, &arr));

    _Invoke(&obj);

    m_socket.Send();

    return true;
}

void Client::SetDSID (const char* DSID_)
{
    utils::MemoryStream* outStream = m_socket.GetOutStream();
    delete[] m_DSID;
    m_DSID = new char[strlen(DSID_)+1];
    strcpy((char*)m_DSID, DSID_);

    // Set the default header
    m_headers.Insert("DSRequestTimeout", AMF3_WRITE_INTEGER_WITH_MARKER(outStream, 60));
    m_headers.Insert("DSId", AMF3_WRITE_STRING_WITH_MARKER(outStream, m_DSID));
    m_headers.Insert("DSEndpoint", AMF3_WRITE_STRING_WITH_MARKER(outStream, "my-rtmps"));
}

void Client::_LoginPart1 ()
{
    utils::MemoryStream* outStream = m_socket.GetOutStream();
    
    // try to get the ip address (honestly no idea why) and the authtoken.
    //_GetIpAddress();
    _GetAuthToken();
    // Send the login credentials and start the real riot server connection
    // Make sure we are working with an empty packet
    OutTypedObject body;
    OutTypedObject wrappedBody;
    char randomUID[37];
    ds::Array<AMF3_FUNCTION> arr(1);
    char macAddress[18];
    Client::GenerateRandomUID(randomUID);
    char hex[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    for (int i = 0; i < 17; i++)
    {
        macAddress[i] = hex[rand()%16];
        if (i == 2 || i == 5 || i == 8 || i == 11 || i == 14)
        {
            macAddress[i] = ':';
        }
    }
    macAddress[17] = '\0';
    

    body.SetType(OutTypedObject::TYPE_OTHER, "com.riotgames.platform.login.AuthenticationCredentials");
    body.Insert("username", AMF3_WRITE_STRING_WITH_MARKER(outStream, m_username));
    body.Insert("password", AMF3_WRITE_STRING_WITH_MARKER(outStream, m_password));
    body.Insert("authToken", AMF3_WRITE_STRING_WITH_MARKER(outStream, m_authToken));
    body.Insert("clientVersion", AMF3_WRITE_STRING_WITH_MARKER(outStream, m_clientVersion));
    //body.Insert("ipAddress", AMF3_WRITE_STRING_WITH_MARKER(outStream, m_currentIpAddress));
    body.Insert("locale", AMF3_WRITE_STRING_WITH_MARKER(outStream, "pt_BR"));
    body.Insert("domain", AMF3_WRITE_STRING_WITH_MARKER(outStream, "lolclient.lol.riotgames.com"));
    body.Insert("macAddress", AMF3_WRITE_STRING_WITH_MARKER(outStream, macAddress));
    body.Insert("operatingSystem", AMF3_WRITE_STRING_WITH_MARKER(outStream, "TEEMO_API"));
    body.Insert("securityAnswer", AMF3_WRITE_NULL(outStream));
    body.Insert("partnerCredentials", AMF3_WRITE_NULL(outStream));
    body.Insert("oldPassword", AMF3_WRITE_NULL(outStream));

    arr.SetElement(0, AMF3_WRITE_OBJECT_WITH_MARKER(outStream, &body));

    _WrapBody(&wrappedBody, "loginService", "login", randomUID, AMF3_WRITE_ARRAY_WITH_MARKER(outStream, &arr));

    _Invoke(&wrappedBody);

    m_socket.Send();
}

void Client::GenerateRandomUID (char* array_)
{
    static const char hex[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    for (int i = 0; i < 36; i++)
    {
        if (i == 8 || i == 13 || i == 18 || i == 23)
        {
            array_[i] = '-';
            continue;
        }
        array_[i] = hex[rand()%16];
    }
    array_[36] = '\0';
}

volatile bool Client::IsConnected ()
{
    return m_isConnected;
}

void Client::SetError (ErrorCode code_)
{
    m_errorCode = code_;
}

void Client::SetError (ErrorCode code_, std::string description_)
{
    m_errorCode = code_;
    m_errorDescription = description_;
}

Client::ErrorCode Client::GetError ()
{
    return m_errorCode;
}

std::string Client::GetErrorDescription ()
{
    return m_errorDescription;
}

void Client::TestClient ()
{
    if (!m_isConnected)
    {
        return;
    }

    utils::MemoryStream* outStream = m_socket.GetOutStream();
    OutTypedObject obj;
    char randomUID[37];
    ds::Array<AMF3_FUNCTION> arr(1);

    Client::GenerateRandomUID(randomUID);

    arr.SetElement(0, AMF3_WRITE_STRING_WITH_MARKER(outStream, "Honux"));
    _WrapBody(&obj, "summonerService", "getSummonerByName", randomUID, AMF3_WRITE_ARRAY_WITH_MARKER(outStream, &arr));

    m_testID = _Invoke(&obj);

    m_socket.Send();
}

bool Client::_DoRMTPSHandshake()
{
    unsigned char randomArray[1528];
    unsigned char S0 = 0;
    unsigned char S1[1536];
    unsigned char S2[1536];
    bool valid = true;
    utils::MemoryStream* outStream = m_socket.GetOutStream();
    utils::MemoryStream* inStream = m_socket.GetInStream();
    // The RMTPS handshake

    outStream->SetCursorPosition(0);

    outStream->WriteU8('\x03');

    outStream->WriteU32((unsigned int)time(NULL));
    outStream->WriteU32(0);
    
    for (int i = 0; i < 1528; i++)
    {
        randomArray[i] = rand()%256;
        outStream->WriteU8(randomArray[i]);
    }

    m_socket.Send();
    m_socket.Receive();

    inStream->ReadU8(&S0);
    if (S0 != '\x03')
    {
        printf("Invalid Handshake version.(%d)", S0);
        return false;
    }

    inStream->ReadData(S1, 1536);

    outStream->SetCursorPosition(0);
    outStream->WriteU32(0);
    outStream->WriteU32((unsigned int)time(NULL));
    outStream->WriteData(S1+8, 1528);

    m_socket.Send();

    inStream->ReadData(S2, 1536);

    for (int i = 8; i < 1536; i++)
    {
        if (randomArray[i-8] != S2[i])
        {
            valid = false;
            break;
        }
    }

    return valid;
}

bool Client::_doConnect()
{
    // Creates the basic connection
    ds::Map<char*, AMF3_FUNCTION> param(7);
    ds::Map<char*, AMF3_FUNCTION> headers(1);
    OutTypedObject to;
    OutTypedObject fool;
    char randomUID[37];
    utils::MemoryStream* outStream = m_socket.GetOutStream();
    char toType[] = "flex.messaging.messages.CommandMessage";
    Client::GenerateRandomUID(randomUID);
    char gameserver[256];

    sprintf(gameserver, "rtmps://%s:%s", g_config.leaguegameServerAddress, g_config.leaguegameServerPort);

    outStream->SetCursorPosition(0);

    param.Insert("app", AMF3_WRITE_STRING_WITH_MARKER(outStream, ""));
    param.Insert("flashVer", AMF3_WRITE_STRING_WITH_MARKER(outStream, "WIN 10,1,85,3"));
    param.Insert("swfUrl", AMF3_WRITE_STRING_WITH_MARKER(outStream, "app:/mod_ser.dat"));
    param.Insert("tcUrl", AMF3_WRITE_STRING_WITH_MARKER(outStream, gameserver));
    param.Insert("fpad", AMF3_WRITE_FALSE(outStream));
    param.Insert("capabilities", AMF3_WRITE_INTEGER_WITH_MARKER(outStream, 239));
    param.Insert("audioCodecs", AMF3_WRITE_INTEGER_WITH_MARKER(outStream, 3191));
    param.Insert("videoCodecs", AMF3_WRITE_INTEGER_WITH_MARKER(outStream, 252));
    param.Insert("videoFunction", AMF3_WRITE_INTEGER_WITH_MARKER(outStream, 1));
    param.Insert("pageUrl", AMF3_WRITE_NULL(outStream));
    param.Insert("objectEncoding", AMF3_WRITE_INTEGER_WITH_MARKER(outStream, 3));

    AMF0::WriteStringWithMarker(outStream, "connect");
    AMF0::WriteDoubleWithMarker(outStream, 1);
    AMF0::WriteAMF3Object(outStream);
    AMF3::WriteAssociativeArrayWithMarker(outStream, &param);
    AMF0::WriteBooleanWithMarker(outStream, false);
    AMF0::WriteStringWithMarker(outStream, "nil");
    AMF0::WriteStringWithMarker(outStream, "");

    to.SetType(OutTypedObject::TYPE_OTHER, toType);
    to.Insert("messageRefType", AMF3_WRITE_NULL(outStream));
    to.Insert("operation", AMF3_WRITE_INTEGER_WITH_MARKER(outStream, 5));
    to.Insert("correlationId", AMF3_WRITE_STRING_WITH_MARKER(outStream, ""));
    to.Insert("clientId", AMF3_WRITE_NULL(outStream));
    to.Insert("destination", AMF3_WRITE_STRING_WITH_MARKER(outStream, ""));
    
    to.Insert("messageId", AMF3_WRITE_STRING_WITH_MARKER(outStream, randomUID));
    to.Insert("timestamp", AMF3_WRITE_INTEGER_WITH_MARKER(outStream, 0));
    to.Insert("timeToLive", AMF3_WRITE_INTEGER_WITH_MARKER(outStream, 0));
    
    to.Insert("body", AMF3_WRITE_OBJECT_WITH_MARKER(outStream, &fool));
    
    headers.Insert("DSMessagingVersion", AMF3_WRITE_INTEGER_WITH_MARKER(outStream, 1));
    headers.Insert("DSId", AMF3_WRITE_STRING_WITH_MARKER(outStream, "my-rtmps"));
    to.Insert("headers", AMF3_WRITE_ASSOCIATIVE_ARRAY_WITH_MARKER(outStream, &headers));

    AMF0::WriteAMF3Object(outStream);
    AMF3::WriteObjectWithMarker(outStream, &to);

    AMF3::AddHeaders(outStream, 0x14);

    m_socket.Send();

    return true;
}

void Client::_GetIpAddress ()
{
    char* request = "GET /services/connection_info HTTP/1.1\x0D\x0AHost: ll.leagueoflegends.com\x0D\x0A\x43onnection: Close\x0D\x0A\x0D\x0A";
    struct hostent *server;
    char buffer[1024];
    size_t myAddrLen = strlen("ll.leagueoflegends.com/services/connection_info");
    struct sockaddr_in servAddr;

    if (m_currentIpAddress != NULL)
    {
        return;
    }

    int sock = socket(AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0)
    {
        delete[] m_currentIpAddress;
        m_currentIpAddress = new char[strlen("10.20.52.241")+1];
        strcpy(m_currentIpAddress, "10.20.52.241");
        return;
    }

    server = gethostbyname("ll.leagueoflegends.com");
    if (server == NULL) 
    {
        delete[] m_currentIpAddress;
        m_currentIpAddress = new char[strlen("10.20.52.241")+1];
        strcpy(m_currentIpAddress, "10.20.52.241");
        return;
    }

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    memcpy((char*)&servAddr.sin_addr.s_addr, (char*)server->h_addr, server->h_length);
    servAddr.sin_port = htons(80);
        
    if (connect(sock,(struct sockaddr *) &servAddr,sizeof(servAddr)) < 0 || send(sock, request, strlen(request), 0) < 0 || recv(sock, buffer, 1024, 0) < 0) 
    {
        delete[] m_currentIpAddress;
        m_currentIpAddress = new char[strlen("10.20.52.241")+1];
        strcpy(m_currentIpAddress, "10.20.52.241");
        return;
    }

    std::string str(strchr(buffer, '{'), strchr(buffer, '}')+1);
    rapidjson::Document ipAddrDocument;
    if (ipAddrDocument.Parse<0>(str.c_str()).HasParseError())
    {
        delete[] m_currentIpAddress;
        m_currentIpAddress = new char[strlen("10.20.52.241")+1];
        strcpy(m_currentIpAddress, "10.20.52.241");
        return;
    }
    
    delete[] m_currentIpAddress;
    m_currentIpAddress = new char[ipAddrDocument["ip_address"].GetStringLength()+1];
    strcpy(m_currentIpAddress, ipAddrDocument["ip_address"].GetString());
    return;
}

void Client::_GetAuthToken ()
{
    boost::asio::io_service io_service;
    boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);
    char request[256];
    sprintf(request,"POST /login-queue/rest/queue/authenticate HTTP/1.1\r\nHost: %s\r\nContent-length: %d\r\n\r\npayload=user%%3D%s%%2Cpassword%%3D%s",
        g_config.leagueLoginServerAddress, 29+strlen(m_username)+strlen(m_password), m_username, m_password);
    SSL_Socket connection(1024, g_config.leagueLoginServerAddress, "443", io_service, ctx);
    int messageSize = 0;
    utils::MemoryStream* inStream = connection.GetInStream();
    std::string jsonData;
    const char* openBracket = NULL;
    const char* buffer = NULL;
    size_t jsonDataSize;
    rapidjson::Document document;

    if (m_authToken != NULL)
    {
        return;
    }

    // Connect, send the request and wait the response
    connection.GetOutStream()->WriteData(request, strlen(request));
    connection.Send();
    do
    {
        messageSize = connection.Receive();
    } while (messageSize == 0);

    buffer = (const char*)inStream->GetBuffer();
    openBracket = strchr(buffer, '{');
    jsonDataSize = (buffer+messageSize)-openBracket;
    jsonData.assign(openBracket, jsonDataSize);

    if (document.Parse<0>(jsonData.c_str()).HasParseError())
    {
        printf("Error While parsing the AuthToken Json.");
        printf("%s\n", jsonData.c_str());
        SetError(ErrorCode::AuthToken_Error);
    }

    if (document.HasMember("token"))
    {
        delete[] m_authToken;

        m_authToken = new char[document["token"].GetStringLength()+1];
        strcpy(m_authToken, document["token"].GetString());
        
        return;
    }

    if (!document["node"].IsInt() || !document["rate"].IsInt() || !document["delay"].IsInt())
    {
        _GetAuthToken();
        return;
    }

    int node = document["node"].GetInt();               // Queue node Id
    char nodeStr[15];
    const char* champ = document["champ"].GetString();  // Queue node Name
    int rate = document["rate"].GetInt();               // Tickets processed per update
    int delay = document["delay"].GetInt();             // Delay between updates
    const rapidjson::Value& tickers = document["tickers"];
    int id;
    int cur;
    
    sprintf(nodeStr, "%d", node);

    for (size_t i = 0; i < tickers.Size(); i++)
    {
        int tnode = tickers[i]["node"].GetInt();
        if (tnode != node)
        {
            continue;
        }

        id = tickers[i]["id"].GetInt(); // Our ticket in line
        cur = tickers[i]["current"].GetInt(); // The current ticket being processed
        break;
    }
    printf("In login queue for %s, #%d in line.\n", m_username, id-cur);

    sprintf(request, "GET /login-queue/rest/queue/ticker/%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", champ, g_config.leagueLoginServerAddress);
    // Request the queue status until there's only 'rate' left to go
    while (id - cur > rate)
    {
        boost::this_thread::sleep_for(boost::chrono::milliseconds(delay));
        SSL_Socket queueConnection(1024, g_config.leagueLoginServerAddress, "443", io_service, ctx);
        rapidjson::Document queueDocument;

        queueConnection.GetOutStream()->WriteData(request, strlen(request));
        queueConnection.Send();
        do
        {
            messageSize = queueConnection.Receive();
        } while (messageSize == 0 || messageSize == -1);
        inStream = queueConnection.GetInStream();

        buffer = (const char*)inStream->GetBuffer();

        openBracket = strchr(buffer, (int)'{');
        jsonDataSize = (buffer+messageSize)-openBracket;
        jsonData.clear();
        jsonData.assign(openBracket, jsonDataSize-1);

        if (queueDocument.Parse<0>(jsonData.c_str()).HasParseError())
        {
            printf("Error While parsing the Login queue Json.");
            printf("%s\n", jsonData.c_str());
            SetError(ErrorCode::LoginQueue_Error);
        }

        const char* hexStr = queueDocument[nodeStr].GetString();
        size_t hexStrLength = queueDocument[nodeStr].GetStringLength();
        cur = 0;
        for (size_t i = 0; i < hexStrLength; i++)
        {
            if (hexStr[i] >= '0' && hexStr[i] <= '9')
            {
                cur = cur * 16 + hexStr[i] - '0';
            }
            else
            {
                cur = cur * 16 + hexStr[i] - 'a' + 10;
            }
        }
        printf("In login queue for %s, #%d in line.\n", m_username, id-cur);
    }
        
    sprintf(request, "GET /login-queue/rest/queue/authToken/%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", m_username, g_config.leagueLoginServerAddress);
    do
    {
        boost::this_thread::sleep_for(boost::chrono::milliseconds(delay));
        SSL_Socket queueConnection(1024, g_config.leagueLoginServerAddress, "443", io_service, ctx);
        rapidjson::Document authTokenDocument;

        queueConnection.GetOutStream()->WriteData(request, strlen(request));
        queueConnection.Send();
        do
        {
            messageSize = queueConnection.Receive();
        } while (messageSize == 0);

        if (messageSize == -1)
        {
            printf("connection on %s aborted, trying to reconnect.\n", m_username);
            continue;
        }

        inStream = queueConnection.GetInStream();

        buffer = (const char*)inStream->GetBuffer();

        openBracket = strchr(buffer, (int)'{');
        jsonDataSize = (buffer+messageSize)-openBracket;
        jsonData.clear();
        jsonData.assign(openBracket, jsonDataSize-1);

        // Parse the json response
        if (authTokenDocument.Parse<0>(jsonData.c_str()).HasParseError())
        {
            continue;
        }
        if (authTokenDocument.HasMember("token"))
        {
            delete[] m_authToken;

            m_authToken = new char[authTokenDocument["token"].GetStringLength()+1];
            strcpy(m_authToken, authTokenDocument["token"].GetString());
            return;
        }
    } while (true);
}

OutTypedObject* Client::_WrapBody (OutTypedObject* target_, const char* destination_, const char* operation_, char* messageId_, AMF3_FUNCTION func_)
{
    utils::MemoryStream* outStream = m_socket.GetOutStream();

    target_->SetType(OutTypedObject::TYPE_OTHER, "flex.messaging.messages.RemotingMessage");
    target_->Insert("destination", AMF3_WRITE_STRING_WITH_MARKER(outStream, destination_));
    target_->Insert("operation", AMF3_WRITE_STRING_WITH_MARKER(outStream, operation_));
    target_->Insert("source", AMF3_WRITE_NULL(outStream));
    target_->Insert("timestamp", AMF3_WRITE_INTEGER_WITH_MARKER(outStream, 0));
    target_->Insert("messageId", AMF3_WRITE_STRING_WITH_MARKER(outStream, messageId_));
    target_->Insert("timeToLive", AMF3_WRITE_INTEGER_WITH_MARKER(outStream, 0));
    target_->Insert("clientId", AMF3_WRITE_NULL(outStream));
    target_->Insert("headers", AMF3_WRITE_OBJECT_WITH_MARKER(outStream, &m_headers));
    target_->Insert("body", func_);

    return target_;
}

uint Client::_Invoke (OutTypedObject* to_)
{
    utils::MemoryStream* outStream = m_socket.GetOutStream();

    outStream->SetCursorPosition(0);
    outStream->WriteU8(0x00);
    outStream->WriteU8(0x05);
    AMF0::WriteDoubleWithMarker(outStream, m_invokeUID);
    outStream->WriteU8(0x05);
    AMF0::WriteAMF3Object(outStream);
    AMF3::WriteObjectWithMarker(outStream, to_);

    AMF3::AddHeaders(outStream, 0x11);

    return m_invokeUID++;
}

void Client::_LoginPart2 (const char* jsonData_)
{
    char randomUID[37];
    char* encodeBuffer;
    char* base64;
    char placeAccountId[14]; // "bc-"+MAX_INTEGER_VALUE
    OutTypedObject obj;
    OutTypedObject empty;
    OutTypedObject specialHeaders;
    ds::Array<AMF3_FUNCTION> objArray(1);
    utils::MemoryStream* outStream = m_socket.GetOutStream();
    Client::GenerateRandomUID(randomUID);
    rapidjson::Document json;

    if (json.Parse<0>(jsonData_).HasParseError())
    {
        SetError(Client::ErrorCode::UnknowError);
        return;
    }
    else if (strcmp("_error", json["result"].GetString()) == 0)
    {
        if (json.HasMember("data") && json["data"].HasMember("rootCause"))
        {
            const rapidjson::Value& rootCause = json["data"]["rootCause"];

            // Missmatch versions, it sends the correct version :D
            if (rootCause.HasMember("errorCode") && strcmp("LOGIN-0001", rootCause["errorCode"].GetString()) == 0)
            {
                SetError(Client::ErrorCode::Wrong_Client_Version, std::string(rootCause["substitutionArguments"][1].GetString()));
            }
            else if (rootCause.HasMember("message"))
            {
                SetError(Client::ErrorCode::UnknowError, std::string(rootCause["message"].GetString()));
            }
            else if (json["data"].HasMember("faultString"))
            {
                SetError(Client::ErrorCode::UnknowError, std::string(json["data"]["faultString"].GetString()));
            }
            else
            {
                SetError(Client::ErrorCode::UnknowError);
            }
        }
        else if (json["data"].HasMember("faultString"))
        {
            SetError(Client::ErrorCode::UnknowError, std::string(json["data"]["faultString"].GetString()));
        }
        else
        {
            SetError(Client::ErrorCode::UnknowError);
        }
        return;
    }

    const rapidjson::Value& body = json["data"]["body"];

    // Get the token
    delete[] m_sessionToken;
    m_sessionToken = new char[body["token"].GetStringLength()+1];
    strcpy(m_sessionToken, body["token"].GetString());

    // Get the account Id
    m_accountId = (int)body["accountSummary"]["accountId"].GetDouble();

    encodeBuffer = new char[strlen(m_username)+strlen(m_sessionToken)+2];
    
    strcpy(encodeBuffer, m_username);
    strcat(encodeBuffer, ":");
    strcat(encodeBuffer, m_sessionToken);

    base64 = new char[Base64encode_len(strlen(encodeBuffer))];
    Base64encode(base64, encodeBuffer, strlen(encodeBuffer));
    
    _WrapBody(&obj, "auth", "8", randomUID, AMF3_WRITE_STRING_WITH_MARKER(outStream, base64));
    obj.SetType(OutTypedObject::TYPE_OTHER, "flex.messaging.messages.CommandMessage");

    _Invoke(&obj);

    m_socket.Send();
    delete[] encodeBuffer;
    delete[] base64;

    // Register ourselves wherever we need to be register
    obj.Clear();
    Client::GenerateRandomUID(randomUID);
    objArray.SetElement(0, AMF3_WRITE_OBJECT_WITH_MARKER(outStream, &empty));

    sprintf(placeAccountId, "bc-%d", m_accountId);

    // Since we are going to use an special type of headers - that is continuously changed, we wont use WrapBody
    // Prepare the body
    specialHeaders.Insert("DSRequestTimeout", AMF3_WRITE_INTEGER_WITH_MARKER(outStream, 60));
    specialHeaders.Insert("DSId", AMF3_WRITE_STRING_WITH_MARKER(outStream, (char*)m_DSID));
    specialHeaders.Insert("DSEndpoint", AMF3_WRITE_STRING_WITH_MARKER(outStream, "my-rtmps"));

    obj.SetType(OutTypedObject::TYPE_OTHER, "flex.messaging.messages.CommandMessage");
    obj.Insert("destination", AMF3_WRITE_STRING_WITH_MARKER(outStream, "messagingDestination"));
    obj.Insert("operation", AMF3_WRITE_STRING_WITH_MARKER(outStream, "0"));
    obj.Insert("source", AMF3_WRITE_NULL(outStream));
    obj.Insert("timestamp", AMF3_WRITE_INTEGER_WITH_MARKER(outStream, 0));
    obj.Insert("messageId", AMF3_WRITE_STRING_WITH_MARKER(outStream, randomUID));
    obj.Insert("timeToLive", AMF3_WRITE_INTEGER_WITH_MARKER(outStream, 0));
    obj.Insert("headers", AMF3_WRITE_OBJECT_WITH_MARKER(outStream, &m_headers));
    obj.Insert("body", AMF3_WRITE_ARRAY_WITH_MARKER(outStream, &objArray));
    /*
    // Subscribe to bc
    specialHeaders.Insert("DSSubtopic", AMF3_WRITE_STRING_WITH_MARKER(outStream, "bc"));
    obj.Insert("clientId", AMF3_WRITE_STRING_WITH_MARKER(outStream, placeAccountId));
    
    client->_Invoke(&obj);
    client->m_socket.Send();

    // Subscribe to cn
    sprintf(placeAccountId, "cn-%d", client->m_accountId);
    specialHeaders.Remove("DSSubtopic");
    obj.Remove("clientId");

    specialHeaders.Insert("DSSubtopic", AMF3_WRITE_STRING_WITH_MARKER(outStream, placeAccountId));
    obj.Insert("clientId", AMF3_WRITE_STRING_WITH_MARKER(outStream, placeAccountId));

    client->_Invoke(&obj);
    client->m_socket.Send();

    // Subscribe to gn
    sprintf(placeAccountId, "gn-%d", client->m_accountId);
    specialHeaders.Remove("DSSubtopic");
    obj.Remove("clientId");

    specialHeaders.Insert("DSSubtopic", AMF3_WRITE_STRING_WITH_MARKER(outStream, placeAccountId));
    obj.Insert("clientId", AMF3_WRITE_STRING_WITH_MARKER(outStream, placeAccountId));

    client->_Invoke(&obj);
    client->m_socket.Send();
    */
    m_isConnected = true;

    // Start the heart beating on this client
    m_heartBeatThread = boost::thread(boost::bind(&Client::_BeatHeart, this));

    printf("Connected: %s.\n", m_username);
}

void ReadData (size_t& bufferLeft_, utils::MemoryStream* stream_, SSL_Socket& socket_, uint8* to_, size_t size_)
{
    if (bufferLeft_ == 0)
    {
        bufferLeft_ = socket_.Receive();
    }

    if (bufferLeft_ > size_)
    {
        bufferLeft_ -= size_;
        stream_->ReadData(to_, size_);
        return;
    }

    stream_->ReadData(to_, bufferLeft_);
    to_ += bufferLeft_;
    size_ -= bufferLeft_;
    bufferLeft_ = 0;

    while (size_ > 0)
    {
        bufferLeft_ = socket_.Receive();
        if (bufferLeft_ > size_)
        {
            bufferLeft_ -= size_;
            stream_->ReadData(to_, size_);
            to_ += size_;
            return;
        }

        stream_->ReadData(to_, bufferLeft_);
        to_ += bufferLeft_;
        size_ -= bufferLeft_;
    }

}

void Client::_ParsePacket ()
{
    uint8 basicHeader;
    int headerType;
    int headerSize = 0;
    uint8 header[16];
    utils::MemoryStream* stream = m_socket.GetInStream();
    size_t bufferLeft = m_socket.Receive();
    Message message;

    for (;;)
    {
        if (bufferLeft == 0)
        {
            bufferLeft = m_socket.Receive();
            if (bufferLeft == 0 || boost::this_thread::interruption_requested())
            {
                return;
            }
        }
        stream->ReadU8(&basicHeader);
        bufferLeft--;
        headerType = basicHeader&0xC0;

        if (headerType == 0x00 || headerType == 0x40)
        {
            ReadData(bufferLeft, stream, m_socket, header, 11-(headerType>>4));
            message.size = ((header[3]&0xFF) << 16)|((header[4]&0xFF) << 8)|(header[5]&0xFF);
            message.type = header[6];
        }
        else
        {
            continue;
        }


        while (message.position != message.size)
        {
            uint length = message.size-message.position;

            if (length > 128)
            {
                length = 128;
            }

            ReadData(bufferLeft, stream, m_socket, message.message+message.position, length);
            message.position += length;

            if (message.position != message.size)
            {
                stream->Forward(1);
                bufferLeft--;
            }
        }

        if (message.type == 0x14)
        {
            utils::MemoryStream realMessage;
            realMessage.Initialize(utils::MemoryStream::ACCESS_READWRITE, message.message, message.size);
            std::ostringstream object;
    
            object << "{";

            object << "\"result\":";
            AMF0::Decode(&realMessage, object, &message);

            object << ",\"invokeId\":";
            AMF0::Decode(&realMessage, object, &message);

            object << ",\"serviceCall\":";
            AMF0::Decode(&realMessage, object, &message);

            object << ",\"data\":";
            AMF0::Decode(&realMessage, object, &message);

            object << "}";

            rapidjson::Document loginJson;
            if (loginJson.Parse<0>(object.str().c_str()).HasParseError() || strcmp("_error", loginJson["result"].GetString()) == 0)
            {
                SetError(Client::ErrorCode::Failed_Json_Login_Data);
            }
            else
            {
                SetDSID(loginJson["data"]["id"].GetString());
                _LoginPart1();
            }
        }
        else if (message.type == 0x11)
        {
            uint8 version;
            utils::MemoryStream realMessage;
            int invokeID;
            std::ostringstream object;
            std::ostringstream invokeIDString;

            realMessage.Initialize(utils::MemoryStream::ACCESS_READWRITE, message.message, message.size);
            realMessage.PeekU8(&version);
            if (version == 0x00)
            {
                realMessage.Forward(1);
            }

            object << "{";

            object << "\"result\":";
            AMF0::Decode(&realMessage, object, &message);
            
            object << ",\"code\":200";

            AMF0::Decode(&realMessage, invokeIDString, &message);
            invokeID = atol(invokeIDString.str().c_str());
            AMF0::Decode(&realMessage, invokeIDString, &message);

            object << ",\"data\":";
            AMF0::Decode(&realMessage, object, &message);

            object << "}";

            
            if (invokeID == 2)
            {
                
                _LoginPart2(object.str().c_str());
            }
            else
            {
                ds::Map<int32, uint32>::Iterator it;
                if (m_callback.Find(invokeID, &it))
                {
                    REQUESTCALLBACK::CreateJsonData (object.str(), it->value);
                    m_callback.RemoveAt(&it);
                }
                else if (m_testID == invokeID)
                {
                    if (object.str().find("Honux") != std::string::npos)
                    {
                        g_testStatus = true;
                    }
                }
            }
        }

        message.Clear();
    }

}

void Client::_BeatHeart ()
{
    while (!boost::this_thread::interruption_requested())
    {
        static const char* weekDay[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
        static const char* month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
        time_t lastBeat = 0;
        size_t beatCounter = 1;

        char timeString[50];
        time_t timeNow = time(NULL);
        struct tm* gmTime = gmtime (&timeNow);

        sprintf(timeString, "%s %s %d %d %d:%d:%d GMT-0300", weekDay[gmTime->tm_wday], month[gmTime->tm_mon], gmTime->tm_mday, gmTime->tm_year+1900, 
            gmTime->tm_hour, gmTime->tm_min, gmTime->tm_sec);

        if (DoBeatHeart(beatCounter, timeString)) 
        {
            lastBeat = timeNow;
            beatCounter++;
        }
        boost::this_thread::sleep_for(boost::chrono::minutes(2));
    }
}