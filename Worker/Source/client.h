#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "string.h"
#include "memorystream.h"
#include "map.h"
#include "SSL_socket.h"
#include "outTypedObject.h"
#include "message.h"
#include "list.h"
#include <boost/thread/thread.hpp>
#include "requestTypes.h"

struct ClassDefinition;

struct RequestThing
{
    RequestType m_type;
    void* m_data;
};

class Client
{
public:
    enum ErrorCode
    {
        No_error                    = 0,
        Wrong_Client_Version        = 1,
        Failed_To_Do_HandShake      = 2,
        AuthToken_Error             = 3,
        LoginQueue_Error            = 4,
        UnknowError                 = 5,
        Failed_Json_Login_Data      = 6
    };

    Client (const char* username_, const char* password_, const char* clientVersion_, boost::asio::io_service& io_service_, boost::asio::ssl::context& ctx_);
    ~Client ();

    // Requests
    void RequestGeneric (const char* destination_, const char* operation_, const char* string_, uint32 taskID_);
    void RequestGeneric (const char* destination_, const char* operation_, int value_, uint32 taskID_);
    void RequestGeneric (const char* destination_, const char* operation_, ds::List<uint32>& numberList_, uint32 taskID_);
    void RequestGeneric (const char* destination_, const char* operation_, ds::List<RequestThing>& thingList_, uint32 taskID_);

    bool DoBeatHeart (uint beatCount_, char* timeString_);

    void SetDSID (const char* DSID_);
    

    static void GenerateRandomUID (char* array_);

    volatile bool IsConnected ();

    void SetError (ErrorCode code_);
    void SetError (ErrorCode code_, std::string description_);
    ErrorCode GetError ();
    std::string GetErrorDescription ();

    void TestClient ();
private:
    bool _DoRMTPSHandshake ();
    bool _doConnect ();
    void _GetIpAddress ();
    void _GetAuthToken ();
    void _ParsePacket ();

    OutTypedObject* _WrapBody (OutTypedObject* target_, const char* destination_, const char* operation_, char* messageId_, AMF3_FUNCTION array_);

    uint _Invoke (OutTypedObject* to_);
    void _LoginPart1 ();
    void _LoginPart2 (const char* jsonData_);
    void _BeatHeart ();
    

    volatile bool m_isConnected;
    int m_accountId;
    uint m_testID;
    ErrorCode m_errorCode;
    std::string m_errorDescription;
    const char* m_username;
    const char* m_password;
    const char* m_clientVersion;
    char* m_DSID;
    char* m_authToken;
    char* m_sessionToken;
    char* m_currentIpAddress;
    uint32 m_invokeUID;
    SSL_Socket m_socket;
    OutTypedObject m_headers;
    boost::thread m_readerThread;
    boost::thread m_heartBeatThread;
    ds::Map<int32, uint32> m_callback;
};

#endif