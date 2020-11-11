#ifndef _CALLBACKS_H_
#define _CALLBACKS_H_
#include "types.h"
#include "rapidjson/document.h"
#include "array.h"

#include <stdio.h>
#include <time.h>
#include <sstream>
#include <fstream>

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/device/back_inserter.hpp>

extern boost::asio::ip::tcp::socket* g_socket;

#define MESSAGE_MAX_SIZE 4096

namespace REQUESTCALLBACK
{
    inline void CreateJsonData (std::string jsonData_, uint32 data_);
};

inline void handle_write(const boost::system::error_code& error)
{}

void REQUESTCALLBACK::CreateJsonData (std::string jsonData_, uint32 taskID_)
{
    if (!g_socket)
    {
        return;
    }

    boost::system::error_code error;

    std::ostringstream gzipedData;
    gzipedData << "0 0 0 0 0";
    boost::iostreams::filtering_istreambuf buff(boost::iostreams::gzip_compressor(8) | boost::make_iterator_range(jsonData_));
    boost::iostreams::copy(buff, gzipedData);

    std::string result = gzipedData.str();
    size_t left = result.length();

    result[0] = 0x01;
    *(uint*)&result[1] = *(uint*)&taskID_;
    *(uint*)&result[5] = left-9;
    const char* buffer = result.c_str();

    do
    {
        uint length = left;

        if (length > MESSAGE_MAX_SIZE)
        {
            length = MESSAGE_MAX_SIZE;
        }

        g_socket->async_send(boost::asio::buffer(buffer, length), boost::bind(&handle_write, boost::asio::placeholders::error));
        buffer += length;
        left -= length;

    } while (left > 0);

}

#endif
