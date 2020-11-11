/********************************************************************//**
  @class utils::MemoryStream
  Defines an interface for a memory stream, that is, a stream that
  operates on memory buffers.
*************************************************************************/

#ifndef _MEMORYSTREAM_H_
#define _MEMORYSTREAM_H_

#include "types.h"

namespace utils
{
    /************************************************************************
      MemoryStreamErrorCode Enum Declaration
    *************************************************************************/
    enum MemoryStreamErrorCode
    {
        MEMORYSTREAM_NO_BUFFER          = -1,
        MEMORYSTREAM_OUT_OF_RANGE       = 1,
        MEMORYSTREAM_END_OF_BUFFER      = 2,
        MEMORYSTREAM_INVALID_OPERATION  = 3,
        MEMORYSTREAM_BUFFER_TOO_SMALL   = 4
    };

    /************************************************************************
      MemoryStream Class Declaration
    *************************************************************************/
    class MemoryStream
    {
    public:

        /************************************************************************
          StreamAccess Enum Declaration
        *************************************************************************/
        enum StreamAccess
        {
            // No access to the stream.
            ACCESS_NONE    = 0,
            // Reading access to the stream.
            ACCESS_READ         = 1,
            // Writing access to the stream.
            ACCESS_WRITE        = 2,
            // Reading and writing access to the stream.
            ACCESS_READWRITE    = (ACCESS_READ | ACCESS_WRITE)
        };

        /// 
        /// Initializes the stream to a dead state.
        /// 
        MemoryStream ();

        /// 
        /// Destroys the stream.
        /// 
        ~MemoryStream ();

    // Properties

        /// 
        /// Gets the access flags of the stream.
        /// @return The access flags of the stream.
        /// 
        uint GetAccess () const;

        /// 
        /// Gets the last error code of the stream.
        /// @return The last error code of the stream.
        /// 
        int GetErrorCode () const;

        /// 
        /// Gets the buffer currently in use by the stream.
        /// @return The buffer currently in use by the stream.
        /// 
        uint8 const* GetBuffer () const;

        /// 
        /// Gets the stream cursor.
        /// @return The stream cursor.
        /// 
        uint8 const* GetCursor () const;

        /// 
        /// Gets the current position of the stream cursor.
        /// @return The current position of the stream cursor.
        /// 
        size_t GetCursorPosition () const ;

        /// 
        /// Sets the current position of the stream cursor.
        /// @param[in] offset Position, relative to the start of the stream, where the
        /// cursor should be changed to.
        /// @return true if the current position of the stream cursor was successfully
        /// changed, false otherwise.
        /// @remarks To get extended error information, call GetLastError.
        /// 
        bool SetCursorPosition (size_t offset) ;

        /// 
        /// Gets a pointer to the end of the stream.
        /// @return Pointer to the end of the stream.
        /// 
        uint8 const* GetEndCursor () const;

        /// 
        /// Gets the position of the end of the stream, which corresponds to the length of the stream.
        /// @return The position of the end of the stream.
        /// 
        size_t GetEndPosition () const ;

        /// 
        /// Sets the position of the end of the stream, which corresponds to the length of the stream.
        /// 
        bool SetEndPosition (size_t endPosition);

    // General functions

        /// 
        /// Initializes the stream.
        /// @param[in] access Desired stream access.
        /// @param[in] begin Pointer to the beginning of the stream.
        /// @param[in] end Pointer to the end of the stream.
        /// @return true if the stream was successfully initialized, false otherwise.
        /// @remarks To get extended error information, call GetLastError.
        /// 
        bool Initialize (uint access, void* begin, void* end);

        /// 
        /// Initializes the stream.
        /// @param[in] access Desired stream access.
        /// @param[in] begin Pointer to the beginning of the stream.
        /// @param[in] size Size, in bytes, of the 'begin' buffer.
        /// @return true if the stream was successfully initialized, false otherwise.
        /// @remarks To get extended error information, call GetLastError.
        /// 
        bool Initialize (uint access, void* begin, size_t size);

        /// 
        /// Checks if the stream can read the specified amount of bytes.
        /// @param[in] count Number of bytes to check.
        /// @return true if the stream can read the specified amount of bytes, false
        /// otherwise.
        /// @remarks To get extended error information, call GetLastError.
        /// 
        bool CanRead (size_t count) const;

        /// 
        /// Checks if the stream can write the specified amount of bytes.
        /// @param[in] count Number of bytes to check.
        /// @return true if the stream can write the specified amount of bytes, false
        /// otherwise.
        /// @remarks To get extended error information, call GetLastError.
        /// 
        bool CanWrite (size_t count) const;

        /// 
        /// Forwards, that is, skips, the specified amount of bytes in the stream.
        /// @param[in] count Amount of bytes to forward the stream in.
        /// @return true if the specified amount of bytes were successfully forwarded, false
        /// otherwise.
        /// @remarks To get extended error information, call GetLastError.
        /// 
         bool Forward (size_t count) ;

        /// 
        /// Rewinds, that is, rollbacks, the specified amount of bytes in the stream.
        /// @param[in] count Amount of bytes to rewind the stream in.
        /// @return true if the specified amount of bytes were successfully rewound, false
        /// otherwise.
        /// @remarks To get extended error information, call GetLastError.
        /// 
         bool Rewind (size_t count) ;

    // Peek functions
    // The peek functions do not move the stream cursor after reading.

        /// 
        /// Peeks an 8-bit unsigned integer from the stream.
        /// @param[out] u8 Pointer to an 8-bit unsigned integer to store the peeked value in.
        /// @return true if the value could be successfully peeked, false otherwise.
        /// @remarks To get extended error information, call GetLastError.
        /// 
         bool PeekU8 (uint8* u8) const;

        /// 
        /// Peeks a 16-bit unsigned integer from the stream.
        /// @param[out] u16 Pointer to a 16-bit unsigned integer to store the peeked value in.
        /// @return true if the value could be successfully peeked, false otherwise.
        /// @remarks To get extended error information, call GetLastError.
        /// 
         bool PeekU16 (uint16* u16) const;

        /// 
        /// Peeks a 32-bit unsigned integer from the stream.
        /// @param[out] u32 Pointer to a 32-bit unsigned integer to store the peeked value in.
        /// @return true if the value could be successfully peeked, false otherwise.
        /// @remarks To get extended error information, call GetLastError.
        /// 
         bool PeekU32 (uint32* u32) const;

        /// 
        /// Peeks a 64-bit unsigned integer from the stream.
        /// @param[out] u64 Pointer to a 64-bit unsigned integer to store the peeked value in.
        /// @return true if the value could be successfully peeked, false otherwise.
        /// @remarks To get extended error information, call GetLastError.
        /// 
         bool PeekU64 (uint64* u64) const;

        /// 
        /// Peeks a 32-bit floating pointer number from the stream.
        /// @param[out] f32 Pointer to a 32-bit floating point number to store the peeked value in.
        /// @return true if the value could be successfully peeked, false otherwise.
        /// @remarks To get extended error information, call GetLastError.
        /// 
         bool PeekFloat (float* f32) const;

        /// 
        /// Peeks a 64-bit floating pointer number from the stream.
        /// @param[out] f64 Pointer to a 64-bit floating point number to store the peeked value in.
        /// @return true if the value could be successfully peeked, false otherwise.
        /// @remarks To get extended error information, call GetLastError.
        /// 
         bool PeekDouble (double* f64) const;

        /// 
        /// Peeks a prefixed-length string from the stream. The string length must be encoded in the
        /// form of two bytes preceding the actual string data.
        /// @param[out] buffer Buffer to store the peeked string in.
        /// @param[in] bufferSize Size of the output buffer.
        /// @return true if the string could be successfully peeked, false otherwise.
        /// @remarks If the buffer does not have enough space to store the whole string,
        /// the function fails and nothing is stored in the buffer. To get extended error
        /// information, call GetLastError.
        /// 
         bool PeekPLString (char* buffer, size_t bufferSize) const;

        /// 
        /// Peeks a string from the stream using the C style, that is, using the
        /// null-terminator as a delimiter.
        /// @param[out] buffer Buffer to store the peeked string is.
        /// @param[in] bufferSize Size of the output buffer.
        /// @return true if the string could be successfully peeked, false otherwise.
        /// @remarks If the buffer does not have enough space to store the whole string,
        /// the function fails and nothing is stored in the buffer. To get extended error
        /// information, call GetLastError.
        /// 
         bool PeekCString (char* buffer, size_t bufferSize) const;

        /// 
        /// Peeks raw data from the stream.
        /// @param[out] data Buffer to store the peeked data in.
        /// @param[in] length Amount of bytes to be read.
        /// @return true if the data could be successfully peeked, false otherwise.
        /// @remarks If the buffer does not have enough space to store the whole data,
        /// the function fails and nothing is stored in the buffer. To get extended error
        /// information, call GetLastError.
        /// 
         bool PeekData (void* data, size_t length) const;

    // Read functions
    // When any of the read functions fail, the stream cursor is left exactly where it
    // was before the call.

        /// 
        /// Reads an 8-bit unsigned integer from the stream.
        /// @param[out] u8 Pointer to an 8-bit unsigned integer to store the read value in.
        /// @return true if the value could be successfully read, false otherwise.
        /// @remarks To get extended error information, call GetLastError.
        /// 
         bool ReadU8 (uint8* u8) ;

        /// 
        /// Reads a 16-bit unsigned integer from the stream.
        /// @param[out] u16 Pointer to a 16-bit unsigned integer to store the read value in.
        /// @return true if the value could be successfully read, false otherwise.
        /// @remarks To get extended error information, call GetLastError.
        /// 
         bool ReadU16 (uint16* u16) ;

        /// 
        /// Reads a 32-bit unsigned integer from the stream.
        /// @param[out] u32 Pointer to a 32-bit unsigned integer to store the read value in.
        /// @return true if the value could be successfully read, false otherwise.
        /// @remarks To get extended error information, call GetLastError.
        /// 
         bool ReadU32 (uint32* u32) ;

        /// 
        /// Reads a 64-bit unsigned integer from the stream.
        /// @param[out] u64 Pointer to a 64-bit unsigned integer to store the read value in.
        /// @return true if the value could be successfully read, false otherwise.
        /// @remarks To get extended error information, call GetLastError.
        /// 
         bool ReadU64 (uint64* u64) ;

        /// 
        /// Reads a 32-bit floating pointer number from the stream.
        /// @param[out] f32 Pointer to a 32-bit floating point number to store the read value in.
        /// @return true if the value could be successfully read, false otherwise.
        /// @remarks To get extended error information, call GetLastError.
        /// 
         bool ReadFloat (float* f32) ;

        /// 
        /// Reads a 64-bit floating pointer number from the stream.
        /// @param[out] f64 Pointer to a 64-bit floating point number to store the read value in.
        /// @return true if the value could be successfully read, false otherwise.
        /// @remarks To get extended error information, call GetLastError.
        /// 
         bool ReadDouble (double* f64) ;

        /// 
        /// Reads a prefixed-length string from the stream. The string length must be encoded in the
        /// form of two bytes preceding the actual string data.
        /// @param[out] buffer Buffer to store the read string in.
        /// @param[in] bufferSize Size of the output buffer.
        /// @return true if the string could be successfully read, false otherwise.
        /// @remarks If the buffer does not have enough space to store the whole string,
        /// the function fails and nothing is stored in the buffer. To get extended error
        /// information, call GetLastError.
        /// 
         bool ReadPLString (char* buffer, size_t bufferSize) ;

        /// 
        /// Reads a string from the stream using the C style, that is, using the
        /// null-terminator as a delimiter.
        /// @param[out] buffer Buffer to store the read string is.
        /// @param[in] bufferSize Size of the output buffer.
        /// @return true if the string could be successfully read, false otherwise.
        /// @remarks If the buffer does not have enough space to store the whole string,
        /// the function fails and nothing is stored in the buffer. To get extended error
        /// information, call GetLastError.
        /// 
         bool ReadCString (char* buffer, size_t bufferSize) ;

        /// 
        /// Reads raw data from the stream.
        /// @param[out] data Buffer to store the read data in.
        /// @param[in] length Amount of bytes to be read.
        /// @return true if the data could be successfully read, false otherwise.
        /// @remarks If the buffer does not have enough space to store the whole data,
        /// the function fails and nothing is stored in the buffer. To get extended error
        /// information, call GetLastError.
        /// 
         bool ReadData (void* data, size_t length) ;

    // Write functions
    // When any of the write functions fail, the stream cursor is left exactly where it
    // was before the call.

        /// 
        /// Writes an 8-bit unsigned integer to the stream.
        /// @param[in] u8 The 8-bit unsigned integer to be written.
        /// @return true if the data could be successfully written, false otherwise.
        /// @remarks To get extended error information, call GetLastError.
        /// 
         bool WriteU8 (uint8 u8) ;

        /// 
        /// Writes an 16-bit unsigned integer to the stream.
        /// @param[in] u16 The 16-bit unsigned integer to be written.
        /// @return true if the data could be successfully written, false otherwise.
        /// @remarks To get extended error information, call GetLastError.
        /// 
         bool WriteU16 (uint16 u16) ;

        /// 
        /// Writes a 32-bit unsigned integer to the stream.
        /// @param[in] u32 The 32-bit unsigned integer to be written.
        /// @return true if the data could be successfully written, false otherwise.
        /// @remarks To get extended error information, call GetLastError.
        /// 
         bool WriteU32 (uint32 u32) ;

        /// 
        /// Writes a 64-bit unsigned integer to the stream.
        /// @param[in] u64 The 64-bit unsigned integer to be written.
        /// @return true if the data could be successfully written, false otherwise.
        /// @remarks To get extended error information, call GetLastError.
        /// 
         bool WriteU64 (uint64 u64) ;

        /// 
        /// Writes a 32-bit floating point number to the stream.
        /// @param[in] f32 The 32-bit floating point number to be written.
        /// @return true if the data could be successfully written, false otherwise.
        /// @remarks To get extended error information, call GetLastError.
        /// 
         bool WriteFloat (float f32) ;

        /// 
        /// Writes a 64-bit floating point number to the stream.
        /// @param[in] f64 The 64-bit floating point number to be written.
        /// @return true if the data could be successfully written, false otherwise.
        /// @remarks To get extended error information, call GetLastError.
        /// 
         bool WriteDouble (double f64) ;

        /// 
        /// Writes a prefixed-length string to the stream. The length is encoded in two bytes
        /// preceding the actual string data. The length must be less than 2^16 - 1.
        /// @param[in] string String to be written to the stream.
        /// @return true if the string could be successfully written, false otherwise.
        /// @remarks If the stream does not have enough space to write the whole string,
        /// the function fails and nothing is written. To get extended error information,
        /// call GetLastError.
        /// 
         bool WritePLString (char const* string) ;

        /// 
        /// Writes a string to the stream using the C style, that is, using the
        /// null-terminator as an end marker.
        /// @param[in] string String to be written to the stream.
        /// @return true if the string could be successfully written, false otherwise.
        /// @remarks If the buffer does not have enough space to write the whole string,
        /// the function fails and nothing is written. To get extended error information,
        /// call GetLastError.
        /// 
         bool WriteCString (char const* string) ;

        /// 
        /// Writes raw data to the stream.
        /// @param[in] data Buffer to be written to the stream.
        /// @param[in] length Amount of bytes to be written.
        /// @return true if the data could be successfully written, false otherwise.
        /// @remarks If the buffer does not have enough space to write the whole data,
        /// the function fails and nothing is written. To get extended error information,
        /// call GetLastError.
        /// 
         bool WriteData (void const* data, size_t length) ;
    
    protected:

        /// 
        /// Sets the access of the stream.
        /// @param[in] access New access of the stream.
        /// 
        void _SetAccess (uint access);

        /// 
        /// Sets the error code of the stream.
        /// @param[in] errorCode New error code of the stream.
        /// 
        void _SetErrorCode (int errorCode) const;

    protected:

        uint m_access;
        mutable int m_errorCode;

    protected:

        uint8* m_buffer;
        uint8* m_cursor;
        uint8* m_end;
    };

    /************************************************************************
      MemoryStream Class Inline Methods
    *************************************************************************/

    inline MemoryStream::~MemoryStream ()
    {
    }

    inline bool MemoryStream::CanRead (size_t count) const
    {
        if (!(m_access & ACCESS_READ))
        {
            _SetErrorCode(MEMORYSTREAM_INVALID_OPERATION);
            return false;
        }
        else if ((m_cursor + count) > m_end)
        {
            _SetErrorCode(MEMORYSTREAM_END_OF_BUFFER);
            return false;
        }

        return true;
    }

    inline bool MemoryStream::CanWrite (size_t count) const
    {
        if (!(m_access & ACCESS_WRITE))
        {
            _SetErrorCode(MEMORYSTREAM_INVALID_OPERATION);
            return false;
        }
        else if ((m_cursor + count) > m_end)
        {
            _SetErrorCode(MEMORYSTREAM_END_OF_BUFFER);
            return false;
        }

        return true;
    }

    inline bool MemoryStream::Forward (size_t count)
    {
        uint8* next = (m_cursor + count);

        if (next > m_end)
        {
            _SetErrorCode(MEMORYSTREAM_OUT_OF_RANGE);
            return false;
        }

        m_cursor = next;
        return true;
    }

    inline uint8 const* MemoryStream::GetBuffer () const
    {
        return m_buffer;
    }

    inline uint8 const* MemoryStream::GetCursor () const
    {
        return m_cursor;
    }

    inline size_t MemoryStream::GetCursorPosition () const
    {
        return (size_t)(m_cursor - m_buffer);
    }

    inline uint8 const* MemoryStream::GetEndCursor () const
    {
        return m_end;
    }

    inline size_t MemoryStream::GetEndPosition () const
    {
        return (size_t)(m_end - m_buffer);
    }

    inline bool MemoryStream::Rewind (size_t count)
    {
        uint8* next = (m_cursor - count);

        if (next < m_buffer)
        {
            _SetErrorCode(MEMORYSTREAM_OUT_OF_RANGE);
            return false;
        }

        m_cursor = next;
        return true;
    }

    inline bool MemoryStream::PeekU8 (uint8* u8) const
    {
        if (!CanRead(sizeof(uint8)))
        {
            return false;
        }

        *u8 = (*m_cursor);
        return true;
    }

    inline bool MemoryStream::PeekU16 (uint16* u16) const
    {
        if (!CanRead(sizeof(uint16)))
        {
            return false;
        }

        *u16 = *(uint16*)m_cursor;
        return true;
    }

    inline bool MemoryStream::PeekU32 (uint32* u32) const
    {
        if (!CanRead(sizeof(uint32)))
        {
            return false;
        }

        *u32 = *(uint32*)m_cursor;
        return true;
    }

    inline bool MemoryStream::PeekU64 (uint64* u64) const
    {
        if (!CanRead(sizeof(uint64)))
        {
            return false;
        }

        *u64 = *(uint64*)m_cursor;
        return true;
    }

    inline bool MemoryStream::PeekFloat (float* f32) const
    {
        if (!CanRead(sizeof(float)))
        {
            return false;
        }

        *f32 = *(float*)m_cursor;
        return true;
    }

    inline bool MemoryStream::PeekDouble (double* f64) const
    {
        if (!CanRead(sizeof(double)))
        {
            return false;
        }

        *f64 = *(double*)m_cursor;
        return true;
    }

    inline bool MemoryStream::ReadU8 (uint8* u8)
    {
        if (!CanRead(sizeof(uint8)))
        {
            return false;
        }

        *u8 = (*m_cursor);
        m_cursor++;
        return true;
    }

    inline bool MemoryStream::ReadU16 (uint16* u16)
    {
        if (!CanRead(sizeof(uint16)))
        {
            return false;
        }

        *u16 = *(uint16*)m_cursor;
        m_cursor += sizeof(uint16);
        return true;
    }

    inline bool MemoryStream::ReadU32 (uint32* u32)
    {
        if (!CanRead(sizeof(uint32)))
        {
            return false;
        }

        *u32 = *(uint32*)m_cursor;
        m_cursor += sizeof(uint32);
        return true;
    }

    inline bool MemoryStream::ReadU64 (uint64* u64)
    {
        if (!CanRead(sizeof(uint64)))
        {
            return false;
        }

        *u64 = *(uint64*)m_cursor;
        m_cursor += sizeof(uint64);
        return true;
    }

    inline bool MemoryStream::ReadFloat (float* f32)
    {
        if (!CanRead(sizeof(float)))
        {
            return false;
        }

        *f32 = *(float*)m_cursor;
        m_cursor += sizeof(float);
        return true;
    }

    inline bool MemoryStream::ReadDouble (double* f64)
    {
        if (!CanRead(sizeof(double)))
        {
            return false;
        }

        *f64 = *(double*)m_cursor;
        m_cursor += sizeof(double);
        return true;
    }

    inline bool MemoryStream::WriteU8 (uint8 u8)
    {
        if (!CanWrite(sizeof(uint8)))
        {
            return false;
        }

        *m_cursor = u8;
        m_cursor++;
        return true;
    }

    inline bool MemoryStream::WriteU16 (uint16 u16)
    {
        if (!CanWrite(sizeof(uint16)))
        {
            return false;
        }

        *(uint16*)m_cursor = u16;
        m_cursor += sizeof(uint16);
        return true;
    }

    inline bool MemoryStream::WriteU32 (uint32 u32)
    {
        if (!CanWrite(sizeof(uint32)))
        {
            return false;
        }

        *(uint32*)m_cursor = u32;
        m_cursor += sizeof(uint32);
        return true;
    }

    inline bool MemoryStream::WriteU64 (uint64 u64)
    {
        if (!CanWrite(sizeof(uint64)))
        {
            return false;
        }

        *(uint64*)m_cursor = u64;
        m_cursor += sizeof(uint64);
        return true;
    }

    inline bool MemoryStream::WriteFloat (float f32)
    {
        if (!CanWrite(sizeof(float)))
        {
            return false;
        }

        *(float*)m_cursor = f32;
        m_cursor += sizeof(float);
        return true;
    }

    inline bool MemoryStream::WriteDouble (double f64)
    {
        if (!CanWrite(sizeof(double)))
        {
            return false;
        }

        *(double*)m_cursor = f64;
        m_cursor += sizeof(double);
        return true;
    }

    inline uint MemoryStream::GetAccess () const
    {
        return m_access;
    }

    inline int MemoryStream::GetErrorCode () const
    {
        return m_errorCode;
    }

    inline void MemoryStream::_SetAccess (uint access)
    {
        m_access = access;
    }

    inline void MemoryStream::_SetErrorCode (int errorCode) const
    {
        m_errorCode = errorCode;
    }

}

#endif
