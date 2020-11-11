#ifndef _AMF0_H_
#define _AMF0_H_

#include <ostream>

namespace utils
{
    class MemoryStream;
};

class Message;

namespace AMF0
{
    void WriteIntWithMarker (utils::MemoryStream* stream_, int value_);
    void WriteInt (utils::MemoryStream* stream_, int value_);

    void WriteDoubleWithMarker (utils::MemoryStream* stream_, double value_);
    void WriteDouble (utils::MemoryStream* stream_, double value_);

    void WriteStringWithMarker (utils::MemoryStream* stream_, const char* value_);
    void WriteString (utils::MemoryStream* stream_, const char* value_);

    void WriteNull (utils::MemoryStream* stream_);
    
    void WriteBooleanWithMarker (utils::MemoryStream* stream_, bool value_);
    void WriteBoolean (utils::MemoryStream* stream_, bool value_);

    void WriteAMF3Object (utils::MemoryStream* stream_);

    void Decode (utils::MemoryStream* stream_, std::ostream& out_, Message* message_);
    void _ReadNumber (utils::MemoryStream* stream_, std::ostream& out_);
    void _ReadBoolean (utils::MemoryStream* stream_, std::ostream& out_);
    void _ReadString (utils::MemoryStream* stream_, std::ostream& out_);
    void _ReadTypedObject (utils::MemoryStream* stream_, std::ostream& out_, Message* message_);
};

#endif