#ifndef _REQUEST_H_
#define _REQUEST_H_

#include "requestTypes.h"
#include "types.h"
#include <string>
#include <vector>

class Task;
class Connection;

struct RequestThing
{
    RequestThing (RequestType type_, const void* data_)
     :m_type(type_),
     m_data(data_)
    {};
    RequestType m_type;
    const void* m_data;
};

class Request
{
public:
    static bool ParseRequest (char* data_, size_t dataLength_, Connection* connection_);

    static void RequestString (const char* destination_, const char* operation_, std::string& string_, Connection* connection_);

    static void RequestNumeric (const char* destination_, const char* operation_, uint32 number_, Connection* connection_);

    static void RequestList (const char* destination_, const char* operation_, std::vector<uint32>& list_, Connection* connection_);

    static void RequestGeneric (const char* destination_, const char* operation_, std::vector<RequestThing>& list_, Connection* connection_);
};

#endif