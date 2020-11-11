#ifndef _AMF3_H_
#define _AMF3_H_

#include "types.h"
#include "map.h"
#include <ostream>

namespace utils
{
    class MemoryStream;
    class String;
};

class OutTypedObject;
class Message;
struct ClassDefinition;

typedef void (*POINTER_TO_AMF3_WRITE)(utils::MemoryStream*, void*);

struct AMF3_FUNCTION
{
    AMF3_FUNCTION(POINTER_TO_AMF3_WRITE function_ = NULL, utils::MemoryStream* stream_ = NULL, void* data_ = NULL)
        :function(function_), m_stream(stream_), m_data(data_)
    {}

    POINTER_TO_AMF3_WRITE function;
    utils::MemoryStream* m_stream;
    void* m_data;

    void Call () const
    {
        if (function)
        {
            function(m_stream, m_data);
        }
    }
};

#define AMF3_WRITE_NULL(stream_) (AMF3_FUNCTION(AMF3::WriteNull, stream_, NULL))
#define AMF3_WRITE_FALSE(stream_) (AMF3_FUNCTION(AMF3::WriteFalse, stream_, NULL))
#define AMF3_WRITE_TRUE(stream_) (AMF3_FUNCTION(AMF3::WriteTrue, stream_, NULL))
#define AMF3_WRITE_BOOL(stream_, value_) (AMF3_FUNCTION(AMF3::WriteBoolean, stream_, (void*)value_))

#define AMF3_WRITE_INTEGER_WITH_MARKER(stream_, value_) (AMF3_FUNCTION(AMF3::WriteIntWithMarker, stream_, (void*)value_))
#define AMF3_WRITE_INTEGER(stream_, value_) (AMF3_FUNCTION(AMF3::WriteInt, stream_, (void*)value_))

#define AMF3_WRITE_DOUBLE_WITH_MARKER(stream_, value_) (AMF3_FUNCTION(AMF3::WriteDoubleWithMarker, stream_, (void*)value_))
#define AMF3_WRITE_DOUBLE(stream_, value_) (AMF3_FUNCTION(AMF3::WriteDouble, stream_, (void*)value_))

#define AMF3_WRITE_STRING_WITH_MARKER(stream_, value_) (AMF3_FUNCTION(AMF3::WriteStringWithMarker, stream_, (void*)value_))
#define AMF3_WRITE_STRING(stream_, value_) (AMF3_FUNCTION(AMF3::WriteString, stream_, (void*)value_))

#define AMF3_WRITE_DATE_WITH_MARKER(stream_, value_) (AMF3_FUNCTION(AMF3::WriteDateWithMarker, stream_, (void*)value_))
#define AMF3_WRITE_DATE(stream_, value_) (AMF3_FUNCTION(AMF3::WriteDate, stream_, (void*)value_))

#define AMF3_WRITE_ARRAY_WITH_MARKER(stream_, value_) (AMF3_FUNCTION(AMF3::WriteArrayWithMarker, stream_, (void*)value_))
#define AMF3_WRITE_ARRAY(stream_, value_) (AMF3_FUNCTION(AMF3::WriteArray, stream_, (void*)value_))

#define AMF3_WRITE_ASSOCIATIVE_ARRAY_WITH_MARKER(stream_, value_) (AMF3_FUNCTION(AMF3::WriteAssociativeArrayWithMarker, stream_, (void*)value_))
#define AMF3_WRITE_ASSOCIATIVE_ARRAY(stream_, value_) (AMF3_FUNCTION(AMF3::WriteAssociativeArray, stream_, (void*)value_))

#define AMF3_WRITE_OBJECT_WITH_MARKER(stream_, value_) (AMF3_FUNCTION(AMF3::WriteObjectWithMarker, stream_, (void*)value_))
#define AMF3_WRITE_OBJECT(stream_, value_) (AMF3_FUNCTION(AMF3::WriteObject, stream_, (void*)value_))


namespace AMF3
{
    void AddHeaders (utils::MemoryStream* stream_, uint8 contentType_ = 0x11);

    void WriteNull (utils::MemoryStream* stream_, void* value_);
    void WriteFalse (utils::MemoryStream* stream_, void* value_);
    void WriteTrue (utils::MemoryStream* stream_, void* value_);
    void WriteBoolean (utils::MemoryStream* stream_, void* value_);

    void WriteIntWithMarker (utils::MemoryStream* stream_, void* value_);
    void WriteInt (utils::MemoryStream* stream_, void* value_);

    void WriteDoubleWithMarker (utils::MemoryStream* stream_, void* value_);
    void WriteDouble (utils::MemoryStream* stream_, void* value_);

    void WriteStringWithMarker (utils::MemoryStream* stream_, void* value_);
    void WriteString (utils::MemoryStream* stream_, void* value_);
    
    void WriteDateWithMarker (utils::MemoryStream* stream_, void* value_);
    void WriteDate (utils::MemoryStream* stream_, void* value_);

    void WriteArrayWithMarker (utils::MemoryStream* stream_, void* value_);
    void WriteArray (utils::MemoryStream* stream_, void* value_);
    
    void WriteAssociativeArrayWithMarker (utils::MemoryStream* stream_, void* value_);
    void WriteAssociativeArray (utils::MemoryStream* stream_, void* value_);
    
    void WriteObjectWithMarker (utils::MemoryStream* stream_, void* value_);
    void WriteObject (utils::MemoryStream* stream_, void* value_);
    
    /*
    static void WriteByteArrayWithMarker (utils::MemoryStream* stream_, const char* array_, uint length_);
    static void WriteByteArray (utils::MemoryStream* stream_, const char* array_, uint length_);
    */

    void Decode (utils::MemoryStream* stream_, std::ostream& out_, Message* message_);
    int _ReadInt (utils::MemoryStream* stream_);
    double _ReadDouble (utils::MemoryStream* stream_);
    std::string _ReadString (utils::MemoryStream* stream_, Message* message_);
    void _ReadDate (utils::MemoryStream* stream_, std::ostream& out_, Message* message_);
    void _ReadArray (utils::MemoryStream* stream_, std::ostream& out_, Message* message_);
    void _ReadObject (utils::MemoryStream* stream_, std::ostream& out_, Message* message_);
    void _ReadByteArray (utils::MemoryStream* stream_, std::ostream& out_, Message* message_);

    void _ReadDSK (utils::MemoryStream* stream_, std::ostream& out_, Message* message_);
    void _ReadDSA (utils::MemoryStream* stream_, std::ostream& out_, Message* message_);
    void _ReadRemaining (int flag_, int bits_, utils::MemoryStream* stream_, Message* message_);
};

#endif