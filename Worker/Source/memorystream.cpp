/************************************************************************
  MemoryStream Class
  Defines an interface for a memory stream, that is, a stream that
  operates on memory buffers.
*************************************************************************/

#include "memorystream.h"

#include "string.h"

#define WIN32_LEAN_AND_MEAN
#include <malloc.h>

#include <cstdio>
#include <cassert>
#include <cstdarg>
#include <cstdlib>

using namespace utils;

/************************************************************************
  MemoryStream Class Public Methods
*************************************************************************/

MemoryStream::MemoryStream ()
{
    m_errorCode = MEMORYSTREAM_NO_BUFFER;
    m_buffer = NULL;
    m_cursor = NULL;
    m_end = NULL;
}

bool MemoryStream::SetCursorPosition (size_t offset)
{
    uint8* cursor = (m_buffer + offset);

    if (cursor > m_end)
    {
        return false;
    }

    m_cursor = cursor;
    return true;
}

bool MemoryStream::SetEndPosition (size_t endPosition)
{
    if (GetCursorPosition() > endPosition)
    {
        return false;
    }
    if (endPosition > GetEndPosition())
    {
        return false;
    }

    m_end = (m_buffer + endPosition);
    return true;
}

bool MemoryStream::Initialize (uint access, void* data, void* end)
{
    if (data == NULL || end == NULL)
    {
        _SetErrorCode(MEMORYSTREAM_NO_BUFFER);
        return false;
    }

    ptrdiff_t size = ((uint8*)end - (uint8*)data);

    if (size <= 0)
    {
        _SetErrorCode(MEMORYSTREAM_NO_BUFFER);
        return false;
    }

    m_buffer = (uint8*)data;
    m_cursor = m_buffer;
    m_end = (uint8*)end;
    _SetAccess(access);
    return true;
}

bool MemoryStream::Initialize (uint access, void* data, size_t size)
{
    if (data == NULL || size == 0)
    {
        _SetErrorCode(MEMORYSTREAM_NO_BUFFER);
        return false;
    }

    m_buffer = (uint8*)data;
    m_cursor = m_buffer;
    m_end = (m_buffer + size);
    _SetAccess(access);
    return true;
}

bool MemoryStream::PeekPLString (char* buffer, size_t bufferSize) const
{
    if (bufferSize == 0)
    {
        _SetErrorCode(MEMORYSTREAM_INVALID_OPERATION);
        return false;
    }

    uint16 strLength;

    if (!PeekU16(&strLength))
    {
        return false;
    }

    if (strLength == 0)
    {
        buffer[0] = '\0';
        return true;
    }

    if ((size_t)strLength >= bufferSize)
    {
        _SetErrorCode(MEMORYSTREAM_BUFFER_TOO_SMALL);
        return false;
    }

    if (!CanRead(strLength + sizeof(uint16)))
    {
        return false;
    }

    memcpy (buffer, (m_cursor + sizeof(uint16)), strLength);

    buffer[strLength] = '\0';
    return true;
}

bool MemoryStream::PeekCString (char* buffer, size_t bufferSize) const
{
    if (!(m_access & ACCESS_READ) || bufferSize == 0)
    {
        _SetErrorCode(MEMORYSTREAM_INVALID_OPERATION);
        return false;
    }

    int remaining = (int)((GetEndPosition() - GetCursorPosition()) & 2147483647);

    if (remaining == 0)
    {
        _SetErrorCode(MEMORYSTREAM_END_OF_BUFFER);
        return false;
    }

    int index = *(int*)(memchr(m_cursor, remaining, '\0'));

    if (index == -1)
    {
        _SetErrorCode(MEMORYSTREAM_END_OF_BUFFER);
        return false;
    }
    if ((size_t)index >= bufferSize)
    {
        _SetErrorCode(MEMORYSTREAM_BUFFER_TOO_SMALL);
        return false;
    }

    if (index > 0)
    {
        memcpy(buffer, m_cursor, (size_t)index);
    }

    buffer[index] = '\0';
    return true;
}

bool MemoryStream::PeekData (void* data, size_t length) const
{
    if (length == 0 || length > 2147483647)
    {
        _SetErrorCode(MEMORYSTREAM_INVALID_OPERATION);
        return false;
    }
    if (!CanRead((uint)length))
    {
        return false;
    }

    memcpy(data, m_cursor, length);
    return true;
}

bool MemoryStream::ReadPLString (char* buffer, size_t bufferSize)
{
    if (bufferSize == 0)
    {
        _SetErrorCode(MEMORYSTREAM_INVALID_OPERATION);
        return false;
    }

    uint16 strLength;

    if (!ReadU16(&strLength))
    {
        return false;
    }

    if (strLength == 0)
    {
        buffer[0] = '\0';
        return true;
    }

    if ((size_t)strLength >= bufferSize)
    {
        _SetErrorCode(MEMORYSTREAM_BUFFER_TOO_SMALL);
        m_cursor -= sizeof(uint16);
        return false;
    }

    if (!CanRead(strLength))
    {
        m_cursor -= sizeof(uint16);
        return false;
    }

    memcpy(buffer, m_cursor, strLength);
    m_cursor += strLength;
    buffer[strLength] = '\0';

    return true;
}

bool MemoryStream::ReadCString (char* buffer, size_t bufferSize)
{
    if (!(m_access & ACCESS_READ) || bufferSize == 0)
    {
        _SetErrorCode(MEMORYSTREAM_INVALID_OPERATION);
        return false;
    }

    int remaining = (int)((GetEndPosition() - GetCursorPosition()) & 2147483647);

    if (remaining == 0)
    {
        _SetErrorCode(MEMORYSTREAM_END_OF_BUFFER);
        return false;
    }

    int index = *(int*)(memchr(m_cursor, remaining, '\0'));

    if (index == -1)
    {
        _SetErrorCode(MEMORYSTREAM_END_OF_BUFFER);
        return false;
    }
    if ((size_t)index >= bufferSize)
    {
        _SetErrorCode(MEMORYSTREAM_BUFFER_TOO_SMALL);
        return false;
    }

    if (index > 0)
    {
        memcpy(buffer, m_cursor, (size_t)index);
    }

    buffer[index] = '\0';
    m_cursor += (index + 1); // Skip null-terminator also.
    return true;
}

bool MemoryStream::ReadData (void* data, size_t length)
{
    if (length == 0 || length > 2147483647)
    {
        _SetErrorCode(MEMORYSTREAM_INVALID_OPERATION);
        return false;
    }
    if (!CanRead((uint)length))
    {
        return false;
    }

    memcpy(data, m_cursor, length);
    m_cursor += length;
    return true;
}

bool MemoryStream::WritePLString (char const* string)
{
    size_t length = strlen(string);

    if (length > 0xFFFF)
    {
        _SetErrorCode(MEMORYSTREAM_INVALID_OPERATION);
        return false;
    }
    if (!CanWrite((uint)(sizeof(uint16) + length)))
    {
        return false;
    }

    WriteU16((uint16)length);

    if (length > 0)
    {
        memcpy(m_cursor, string, length);
        m_cursor += length;
    }

    return true;
}

bool MemoryStream::WriteCString (char const* string)
{
    uint length = (uint)(strlen(string) + 1); // Include null-terminator.

    if (!CanWrite(length))
    {
        return false;
    }

    memcpy(m_cursor, string, length);
    m_cursor += length;
    return true;
}

bool MemoryStream::WriteData (void const* data, size_t length)
{
    if (length == 0 || length > 2147483647)
    {
        _SetErrorCode(MEMORYSTREAM_INVALID_OPERATION);
        return false;
    }
    if (!CanWrite((uint)length))
    {
        return false;
    }

    memcpy(m_cursor, data, length);
    m_cursor += length;
    return true;
}
