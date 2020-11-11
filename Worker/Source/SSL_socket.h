#ifndef __SSL_SOCKET_H__
#define __SSL_SOCKET_H__

#include "types.h"
#include "memorystream.h"
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <fstream>

class SSL_Socket
{
public:
    SSL_Socket (uint32 bufferSize_, const char* serverAddr_, const char* port_, boost::asio::io_service& io_service_, boost::asio::ssl::context& context_)
    :m_bufferSize(bufferSize_),
      m_socket(io_service_, context_),
      m_isConnected(false)
    {
        m_inBuffer = new uint8[bufferSize_];
        m_outBufer = new uint8[bufferSize_];
        memset(m_inBuffer, 0, bufferSize_);
        memset(m_outBufer, 0, bufferSize_);

        m_inStream.Initialize(utils::MemoryStream::ACCESS_READWRITE, m_inBuffer, bufferSize_);
        m_outStream.Initialize(utils::MemoryStream::ACCESS_READWRITE, m_outBufer, bufferSize_);
    
        Connect(serverAddr_, port_);
    }

    ~SSL_Socket ()
    {
        boost::system::error_code ignored;
        m_socket.shutdown(ignored);
        delete [] m_outBufer;
        delete [] m_inBuffer;
    }
   
    void Connect (const char* serverAddr_, const char* port_)
    {
        boost::system::error_code ignored;
        m_socket.shutdown(ignored);

        boost::asio::ip::tcp::resolver resolver(m_socket.get_io_service());
        boost::asio::ip::tcp::resolver::query query(serverAddr_, port_);
        boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        
        // We are not going to check the certificates
        m_socket.set_verify_mode(boost::asio::ssl::verify_none);
        
        boost::system::error_code ec;
        boost::asio::connect(m_socket.lowest_layer(), endpoint_iterator, ec);
        if (ec)
        {
            m_isConnected = false;
        }
        m_socket.handshake(boost::asio::ssl::stream_base::handshake_type::client);
        m_isConnected = true;
    }

    int Send ()
    {
        boost::system::error_code ec;
        int bytesSent = m_socket.write_some(boost::asio::buffer(m_outBufer, m_outStream.GetCursorPosition()), ec);
        std::fstream fs;
        fs.open("out2.txt", std::fstream::in | std::fstream::out);

        for (int i = 0; i < m_outStream.GetCursorPosition(); i++)
        {
            fs << (int)m_outBufer[i] << " ";
            if (i % 10 == 0)
                fs << "\n";
        }

        fs.close();
        m_outStream.SetCursorPosition(0);
        return bytesSent;
    }

    int Receive ()
    {
        boost::system::error_code ec;
        //int bytesReceived = m_socket.read_some(boost::asio::buffer(m_inBuffer, m_bufferSize-1), ec);
        int bytesReceived = boost::asio::read(m_socket, boost::asio::buffer(m_inBuffer, m_bufferSize-1), boost::asio::transfer_at_least(1), ec);
        m_inStream.Initialize(utils::MemoryStream::ACCESS_READWRITE, m_inBuffer, bytesReceived);
        return bytesReceived;
    }

    utils::MemoryStream* GetInStream ()
    {
        return &m_inStream;
    }

    utils::MemoryStream* GetOutStream ()
    {
        return &m_outStream;
    }

private:
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket> m_socket;
    uint32 m_bufferSize;
    uint8* m_outBufer;
    uint8* m_inBuffer;
    utils::MemoryStream m_inStream;
    utils::MemoryStream m_outStream;
    bool m_isConnected;
};

#endif