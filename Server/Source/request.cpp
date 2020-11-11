#include "request.h"
#include <string>
#include "taskHolder.h"
#include "requestTypes.h"
#include "workers.h"
#include "worker.h"
#include "task.h"
#include "connection.h"
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>

bool Request::ParseRequest (char* data_, size_t dataLength_, Connection* connection_)
{
    char* ptr = data_;
    char* end = data_+dataLength_-1;
    size_t left = dataLength_;

    if (left >= 8 && strncmp(ptr, "/player/", 8) == 0)
    {
        size_t playerNameLength = 0;
        ptr += 8;
        left -= 8;
        while (end != ptr+playerNameLength)
        {
            if (*(ptr+playerNameLength) == '/')
            {
                break;
            }
            if (*(ptr+playerNameLength) == ' ' && *(ptr+playerNameLength+1) == 'H' && *(ptr+playerNameLength+2) == 'T' && *(ptr+playerNameLength+3) == 'T' && *(ptr+playerNameLength+4) == 'P')
            {
                break;
            }
            playerNameLength++;
        }
        std::string playerName(ptr, playerNameLength);
        ptr += playerNameLength;
        left -= playerNameLength;

        boost::replace_all(playerName, "%20", " ");

        if (left >= 5 && strncmp(ptr, " HTTP", 5) == 0 || left >= 6 && strncmp(ptr+1, " HTTP", 5) == 0)
        {
            RequestString("summonerService", "getSummonerByName", playerName, connection_);
            return true;
        }
        else if (left >= 7 && strncmp(ptr, "/inGame", 7) == 0)
        {
            RequestString("gameService", "retrieveInProgressSpectatorGameInfo", playerName, connection_);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (left >= 11 && strncmp(ptr, "/accountid/", 11) == 0)
    {
        size_t accountIDLength = 0;
        ptr += 11;
        left -= 11;
        while (end != ptr+accountIDLength)
        {
            if (*(ptr+accountIDLength) == '/')
            {
                break;
            }
            if (*(ptr+accountIDLength) == ' ' && *(ptr+accountIDLength+1) == 'H' && *(ptr+accountIDLength+2) == 'T' && *(ptr+accountIDLength+3) == 'T' && *(ptr+accountIDLength+4) == 'P')
            {
                return false;
                break;
            }
            accountIDLength++;
        }
        uint32 accountID = 0;
        try
        {
            accountID = boost::lexical_cast<uint32>(ptr, accountIDLength);
        }
        catch(boost::bad_lexical_cast)
        { 
            return false; 
        }

        ptr += accountIDLength;
        left -= accountIDLength;

        if (left >= 12 && strncmp(ptr, "/recentGames", 12) == 0)
        {
            RequestNumeric("playerStatsService", "getRecentGames", accountID, connection_);
            return true;
        }
        else if (left >= 14 && strncmp(ptr, "/allPublicData", 14) == 0)
        {
            RequestNumeric("summonerService", "getAllPublicSummonerDataByAccount", accountID, connection_);
            return true;
        }
        else if (left >= 6 && strncmp(ptr, "/stats", 6) == 0)
        {
            RequestNumeric("playerStatsService", "retrievePlayerStatsByAccountId", accountID, connection_);
            return true;
        }
        else if (left >= 6 && strncmp(ptr, "/topPlayed", 6) == 0)
        {
            std::vector<RequestThing> list;
            list.push_back(RequestThing(RequestType::Numeric_Request, (void*)accountID));
            list.push_back(RequestThing(RequestType::String_Request, "CLASSIC"));
            RequestGeneric("playerStatsService", "retrieveTopPlayedChampions", list, connection_);
            return true;
        }
        else if (left >= 13 && strncmp(ptr, "/rankedStats/", 13) == 0)
        {
            uint32 season = 0;
            try
            {
                season = boost::lexical_cast<uint32>(ptr+13, 1);
            }
            catch(boost::bad_lexical_cast)
            { 
                return false; 
            }

            std::vector<RequestThing> list;
            list.push_back(RequestThing(RequestType::Numeric_Request, (void*)accountID));
            list.push_back(RequestThing(RequestType::String_Request, "CLASSIC"));
            list.push_back(RequestThing(RequestType::Numeric_Request, (void*)season));

            RequestGeneric("playerStatsService", "getAggregatedStats", list, connection_);
            return true;
        }
    }
    else if (left >= 12 && strncmp(ptr, "/summonerid/", 12) == 0)
    {
        size_t summonerIDLength = 0;
        ptr += 12;
        left -= 12;
        while (end != ptr+summonerIDLength)
        {
            if (*(ptr+summonerIDLength) == '/')
            {
                break;
            }
            if (*(ptr+summonerIDLength) == ' ' && *(ptr+summonerIDLength+1) == 'H' && *(ptr+summonerIDLength+2) == 'T' && *(ptr+summonerIDLength+3) == 'T' && *(ptr+summonerIDLength+4) == 'P')
            {
                return false;
                break;
            }
            summonerIDLength++;
        }

        uint32 summonerID = 0;
        try
        {
            summonerID = boost::lexical_cast<uint32>(ptr, summonerIDLength);
        }
        catch(boost::bad_lexical_cast)
        { 
            return false; 
        }
        
        ptr += summonerIDLength;
        left -= summonerIDLength;

        if (left >= 8 && strncmp(ptr, "/leagues", 8) == 0)
        {
            RequestNumeric("leaguesServiceProxy", "getAllLeaguesForPlayer", summonerID, connection_);
            return true;
        }
        else if (left >= 6 && strncmp(ptr, "/honor", 6) == 0)
        {
            std::string jsonString("{\"commandName\":\"TOTALS\",\"summonerId\":");
            jsonString += boost::lexical_cast<std::string>(summonerID);
            jsonString += "}";

            RequestString("clientFacadeService", "callKudos", jsonString, connection_);
            return true;
        }
        else if (left >= 6 && strncmp(ptr, "/runes", 6) == 0)
        {
            RequestNumeric("spellBookService", "getSpellBook", summonerID, connection_);
            return true;
        }
        else if (left >= 10 && strncmp(ptr, "/masteries", 10) == 0)
        {
            RequestNumeric("masteryBookService", "getMasteryBook", summonerID, connection_);
            return true;
        }
    }
    else if (left >= 6 && strncmp(ptr, "/list/", 6) == 0)
    {
        std::vector<uint32> list;
        size_t summonerIDLength = 0;
        ptr += 6;
        left -= 6;
        while (summonerIDLength++ <= left)
        {
            if (*(ptr+summonerIDLength) == ';')
            {
                try
                {
                    list.push_back(boost::lexical_cast<uint32>(ptr, summonerIDLength));
                }
                catch(boost::bad_lexical_cast)
                { 
                    return false; 
                }

                ptr += summonerIDLength+1;
                left -= summonerIDLength+1;
                summonerIDLength = 0;
            }
            else if (*(ptr+summonerIDLength) == '/')
            {
                try
                {
                    list.push_back(boost::lexical_cast<uint32>(ptr, summonerIDLength));
                }
                catch(boost::bad_lexical_cast)
                { 
                    return false; 
                }

                ptr += summonerIDLength;
                left -= summonerIDLength;
                break;
            }
            if (*(ptr+summonerIDLength) == ' ' && *(ptr+summonerIDLength+1) == 'H' && *(ptr+summonerIDLength+2) == 'T' && *(ptr+summonerIDLength+3) == 'T' && *(ptr+summonerIDLength+4) == 'P')
            {
                return false;
                break;
            }
        }

        if (list.size() > 30)
        {
            return false;
        }
        if (left >= 6 && strncmp(ptr, "/icons", 6) == 0)
        {
            RequestList("summonerService", "getSummonerIcons", list, connection_);
            return true;
        }
        else if (left >= 6 && strncmp(ptr, "/names", 6) == 0)
        {
            RequestList("summonerService", "getSummonerNames", list, connection_);
            return true;
        }
    }
    else if (left >= 7 && strncmp(ptr, "/server", 7) == 0)
    {
        ptr += 7;
        left -= 7;
        if (left >= 7 && strncmp(ptr, "/status", 7) == 0)
        {
            std::string workersData = Workers::GetInstance().GetWorkersInformation();
            std::string result("HTTP/1.1 200 OK\r\nContent-Length: ");
            result.append(boost::lexical_cast<std::string>(workersData.size()));
            result.append("\r\n"
                         "Content-Type: application/json\r\n"
                         "Connection: close\r\n"
                         "\r\n");
            result.append(workersData);
            result.append("\r\n");
            connection_->SendAndRelease(result.c_str(), result.size());
            return true;
        }
        else if (left >= 7 && strncmp(ptr, "/worker", 7) == 0)
        {
            ptr += 8;
            left -= 8;
            size_t workerIDLength = 0;
            while (true)
            {
                if (*(ptr+workerIDLength) >= '0' && *(ptr+workerIDLength) <= '9')
                {
                    ++workerIDLength;
                }
                else
                {
                    break;
                }
            }
            uint32 position = 0;
            try
            {
                position = boost::lexical_cast<uint32>(ptr, workerIDLength);
            }
            catch (boost::bad_lexical_cast)
            { 
                return false; 
            }

            Worker* worker = Workers::GetInstance().GetWorkerAtPosition(position);
            ptr += workerIDLength;

            if (worker)
            {
                if (left >= 5 && strncmp(ptr, "/test", 5) == 0)
                {
                    char buffer[1024] = {RequestType::String_Request, 0};
                    size_t offset = 5;
                    *(uint*)&buffer[1] = TaskHolder::GetInstance().CreateTask("summonerService", "getSummonerByName", connection_)->GetTaskID();
                    // Copies the destination
                    buffer[offset] = strlen("summonerService");
                    memcpy(buffer+offset+1, "summonerService", buffer[offset]+1);
                    offset += buffer[offset]+2;
                    // Copies the operation
                    buffer[offset] = strlen("getSummonerByName");
                    memcpy(buffer+offset+1, "getSummonerByName", buffer[offset]+1);
                    offset += buffer[offset]+2;
                    // Copies the string
                    buffer[offset] = strlen("Honux");
                    memcpy(buffer+offset+1, "Honux", buffer[offset]+1);
                    offset += buffer[offset]+1;

                    worker->SendData(buffer, offset);
                    return true;
                }
                else if (left >= 8 && strncmp(ptr, "/restart", 8) == 0)
                {
                    char buffer[20] = {(char)RequestType::Force_Reconnect, 0};
                    worker->SendData(buffer, 20);
                    const char success[] = "HTTP/1.1 200 OK\r\n"
                    "Content-Length: 72\r\n"
                    "Content-Type: application/json\r\n"
                    "Connection: close\r\n"
                    "\r\n"
                    "{\"success\":true, \"code\":200, \"data\":{\"message\":\"Worker is restarting.\"}}\r\n";
                    connection_->SendAndRelease(success, strlen(success));
                    return true;
                }
                else if (left >= 5 && strncmp(ptr, "/kill", 5) == 0)
                {
                    char buffer[20] = {(char)RequestType::Kill, 0};
                    worker->SendData(buffer, 20);
                    const char success[] = "HTTP/1.1 200 OK\r\n"
                    "Content-Length: 84\r\n"
                    "Content-Type: application/json\r\n"
                    "Connection: close\r\n"
                    "\r\n"
                    "{\"success\":true, \"code\":200, \"data\":{\"message\":\"Killed the worker. List updated.\"}}\r\n";
                    connection_->SendAndRelease(success, strlen(success));

                    Workers::GetInstance().UnsubscribeWorker(worker->GetUniqueID());
                    return true;
                }
            }
            else
            {
                const char service_unavailable[] = "HTTP/1.1 503 Service Unavailable\r\n"
                    "Content-Length: 67\r\n"
                    "Content-Type: application/json\r\n"
                    "Connection: close\r\n"
                    "\r\n"
                    "{\"success\":false, \"code\":503, \"data\":{\"error\":\"Worker not found.\"}}\r\n";
                connection_->SendAndRelease(service_unavailable, strlen(service_unavailable));
                return true;
            }
        }
    }
    else if (left >= 8 && strncmp(ptr, "/numeric", 8) == 0)
    {
        ptr += 9;
        left -= 9;

        // Get the number
        size_t numberLength = 0;
        while (end != ptr+numberLength)
        {
            if (*(ptr+numberLength) == '/')
            {
                break;
            }
            numberLength++;
        }

        uint32 number = 0;
        try
        {
            number = boost::lexical_cast<uint32>(ptr, numberLength);
        }
        catch(boost::bad_lexical_cast)
        { 
            return false; 
        }
        
        ptr += numberLength+1;
        left -= numberLength+1;

        // Destination
        size_t destinationLength = 0;
        while (end != ptr+destinationLength)
        {
            if (*(ptr+destinationLength) == '/')
            {
                break;
            }
            destinationLength++;
        }
        std::string destination(ptr, destinationLength);
        ptr += destinationLength+1;
        left -= destinationLength+1;

        boost::replace_all(destination, "%20", " ");

        // Service
        size_t operationLength = 0;
        while (end != ptr+operationLength)
        {
            if (*(ptr+operationLength) == '/')
            {
                break;
            }
            if (*(ptr+operationLength) == ' ' && *(ptr+operationLength+1) == 'H' && *(ptr+operationLength+2) == 'T' && *(ptr+operationLength+3) == 'T' && *(ptr+operationLength+4) == 'P')
            {
                break;
            }
            operationLength++;
        }
        std::string operation(ptr, operationLength);
        ptr += operationLength+1;
        left -= operationLength+1;

        boost::replace_all(operation, "%20", " ");
        RequestNumeric(destination.c_str(), operation.c_str(), number, connection_);
        return true;
    }
    return false;
}

void Request::RequestString (const char* destination_, const char* operation_, std::string& string_, Connection* connection_)
{
    char buffer[1024] = {RequestType::String_Request, 0};
    size_t offset = 5;
    *(uint*)&buffer[1] = TaskHolder::GetInstance().CreateTask(destination_, operation_, connection_)->GetTaskID();
    // Copies the destination
    buffer[offset] = strlen(destination_);
    memcpy(buffer+offset+1, destination_, buffer[offset]+1);
    offset += buffer[offset]+2;
    // Copies the operation
    buffer[offset] = strlen(operation_);
    memcpy(buffer+offset+1, operation_, buffer[offset]+1);
    offset += buffer[offset]+2;
    // Copies the string
    buffer[offset] = string_.length();
    memcpy(buffer+offset+1, string_.c_str(), buffer[offset]+1);
    offset += buffer[offset]+1;

    Workers::GetInstance().GetAvaiableWorker()->SendData(buffer, offset);
}

void Request::RequestNumeric (const char* destination_, const char* operation_, uint32 number_, Connection* connection_)
{
    char buffer[1024] = {RequestType::Numeric_Request, 0};
    size_t offset = 5;
    *(uint*)&buffer[1] = TaskHolder::GetInstance().CreateTask(destination_, operation_, connection_)->GetTaskID();
    // Copies the destination
    buffer[offset] = strlen(destination_);
    memcpy(buffer+offset+1, destination_, buffer[offset]+1);
    offset += buffer[offset]+2;
    // Copies the operation
    buffer[offset] = strlen(operation_);
    memcpy(buffer+offset+1, operation_, buffer[offset]+1);
    offset += buffer[offset]+2;
    // Copies the number
    *(uint32*)&buffer[offset] = number_;
    offset += 4;

    Workers::GetInstance().GetAvaiableWorker()->SendData(buffer, offset);
}

void Request::RequestList (const char* destination_, const char* operation_, std::vector<uint32>& list_, Connection* connection_)
{
    char buffer[1024] = {RequestType::List_Request, 0};
    size_t offset = 5;
    *(uint*)&buffer[1] = TaskHolder::GetInstance().CreateTask(destination_, operation_, connection_)->GetTaskID();
    // Copies the destination
    buffer[offset] = strlen(destination_);
    memcpy(buffer+offset+1, destination_, buffer[offset]+1);
    offset += buffer[offset]+2;
    // Copies the operation
    buffer[offset] = strlen(operation_);
    memcpy(buffer+offset+1, operation_, buffer[offset]+1);
    offset += buffer[offset]+2;
    // Copies the list size
    buffer[offset++] = list_.size();
    // Copies the list elements
    for (std::vector<uint32>::const_iterator it = list_.begin(); it != list_.end(); it++)
    {
        *(uint32*)&buffer[offset] = *it;
        offset += 4;
    }

    Workers::GetInstance().GetAvaiableWorker()->SendData(buffer, offset);
}

void Request::RequestGeneric (const char* destination_, const char* operation_, std::vector<RequestThing>& list_, Connection* connection_)
{
    char buffer[1024] = {RequestType::Generic_Request, 0};
    size_t offset = 5;
    *(uint*)&buffer[1] = TaskHolder::GetInstance().CreateTask(destination_, operation_, connection_)->GetTaskID();
    // Copies the destination
    buffer[offset] = strlen(destination_);
    memcpy(buffer+offset+1, destination_, buffer[offset]+1);
    offset += buffer[offset]+2;
    // Copies the operation
    buffer[offset] = strlen(operation_);
    memcpy(buffer+offset+1, operation_, buffer[offset]+1);
    offset += buffer[offset]+2;
    // Copies the list size
    buffer[offset++] = list_.size();
    // Copies the list elements
    for (std::vector<RequestThing>::const_iterator it = list_.begin(); it != list_.end(); it++)
    {
        buffer[offset++] = (*it).m_type;
        if ((*it).m_type == RequestType::Numeric_Request)
        {
            uint32 fool = *(uint32*)&(*it).m_data;
            *(uint32*)&buffer[offset] = fool;
            offset += 4;
        }
        else
        {
            buffer[offset] = strlen((char*)(*it).m_data)+1;
            memcpy(&buffer[offset+1], (*it).m_data, buffer[offset]);
            offset += buffer[offset]+1;
        }
    }

    Workers::GetInstance().GetAvaiableWorker()->SendData(buffer, offset);
}