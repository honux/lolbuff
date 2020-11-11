#include "amf0.h"
#include "bigEndian.h"
#include "memorystream.h"
#include "amf3.h"
#include "message.h"

#include <iomanip>
#include <boost/algorithm/string/replace.hpp>

void AMF0::WriteIntWithMarker (utils::MemoryStream* stream_, int value_)
{
    WriteDoubleWithMarker(stream_, value_);
}

void AMF0::WriteInt (utils::MemoryStream* stream_, int value_)
{
    WriteDouble(stream_, value_);
}

void AMF0::WriteDoubleWithMarker (utils::MemoryStream* stream_, double value_)
{
    stream_->WriteU8(utils::BigEndianU8(0x00));
    WriteDouble(stream_, value_);
}

void AMF0::WriteDouble (utils::MemoryStream* stream_, double value_)
{
    stream_->WriteDouble(utils::BigEndianDouble(value_));
}

void AMF0::WriteStringWithMarker (utils::MemoryStream* stream_, const char* value_)
{
    stream_->WriteU8(utils::BigEndianU8(0x02));
    WriteString(stream_, value_);
}

void AMF0::WriteString (utils::MemoryStream* stream_, const char* value_)    
{
    uint16 length = (uint16)strlen(value_);
    stream_->WriteU16(utils::BigEndianU16(length));
    if (length > 0)
    {
        stream_->WriteData(value_, length);
    }
}

void AMF0::WriteNull (utils::MemoryStream* stream_)
{
    stream_->WriteU8(utils::BigEndianU8(0x05));
}

void AMF0::WriteBooleanWithMarker (utils::MemoryStream* stream_, bool value_)
{
    stream_->WriteU8(utils::BigEndianU8(0x01));
    WriteBoolean(stream_, value_);
}

void AMF0::WriteBoolean (utils::MemoryStream* stream_, bool value_)
{
    if (value_)
    {
        stream_->WriteU8(utils::BigEndianU8(0x01));
    }
    else
    {
        stream_->WriteU8(utils::BigEndianU8(0x00));
    }
}

void AMF0::WriteAMF3Object (utils::MemoryStream* stream_)
{
    stream_->WriteU8(utils::BigEndianU8(0x11));
}

void AMF0::Decode (utils::MemoryStream* stream_, std::ostream& out_, Message* message_)
{
    uint8 type;
    stream_->ReadU8(&type);

    switch (type)
    {
        case 0x00:
            _ReadNumber(stream_, out_);
        break;

        case 0x01:
            _ReadBoolean(stream_, out_);
        break;

        case 0x02:
            _ReadString(stream_, out_);
        break;

        case 0x03:
            _ReadTypedObject(stream_, out_, message_);
        break;

        case 0x05:
            out_ << "null";
        break;

        case 0x011:
            AMF3::Decode(stream_, out_, message_);
        break;
    }
}

void AMF0::_ReadNumber (utils::MemoryStream* stream_, std::ostream& out_)
{
    double value;
    stream_->ReadDouble(&value);
    value = utils::BigEndianDouble(value);
    out_ << value;
}

void AMF0::_ReadBoolean (utils::MemoryStream* stream_, std::ostream& out_)
{
    uint8 fool;
    stream_->ReadU8(&fool);

    if (fool==0)
    {
        out_ << "false";
    }
    else
    {
        out_ << "true";
    }
}

void AMF0::_ReadString (utils::MemoryStream* stream_, std::ostream& out_)
{
    uint16 strLen;

    stream_->ReadU16(&strLen);
    strLen = utils::BigEndianU16(strLen);
    std::string data((const char*)stream_->GetCursor(), strLen);

    boost::replace_all(data, "\\", "\\\\");
    boost::replace_all(data, "\b", "\\\\b");
    boost::replace_all(data, "\f", "\\\\f");
    boost::replace_all(data, "\n", "\\\\n");
    boost::replace_all(data, "\r", "\\\\r");
    boost::replace_all(data, "\t", "\\\\t");
    boost::replace_all(data, "\v", "\\\\v");
    boost::replace_all(data, "\"", "\\\"");

    out_ << "\"";
    out_ << data;
    out_ << "\"";
    stream_->Forward(strLen);
}

void AMF0::_ReadTypedObject (utils::MemoryStream* stream_, std::ostream& out_, Message* message_)
{
    uint16 strLen;
    bool first = true;

    out_ << "{";
    while (true)
    {
        stream_->ReadU16(&strLen);
        strLen = utils::BigEndianU16(strLen);
        if (strLen == 0)
        {
            break;
        }

        if (first)
        {
            first = false;
        }
        else
        {
            out_ << ",";
        }

        out_ << "\"";
        out_.write((const char*)stream_->GetCursor(), strLen);
        out_ << "\":";

        stream_->Forward(strLen);

        AMF0::Decode(stream_, out_, message_);
    }
    out_ << "}";
    stream_->Forward(1);
}
