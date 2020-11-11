#include "amf3.h"
#include "memorystream.h"
#include "bigEndian.h"
#include "outTypedObject.h"
#include <time.h>
#include "array.h"
#include "classDefinition.h"
#include "message.h"
#define ABNF28BITINTEGER(value_) ((value_<<1)|1)

#include <sstream>
#include <iomanip>
#include <boost/algorithm/string/replace.hpp>

void AMF3::AddHeaders (utils::MemoryStream* stream_, uint8 contentType_)
{
    static time_t startTime = time(NULL);

    size_t lenght = stream_->GetCursorPosition();
    char* content = new char[lenght];
    time_t timediff = (uint32)time(NULL)-startTime;
    memcpy(content, stream_->GetBuffer(), lenght);

    stream_->SetCursorPosition(0);

    stream_->WriteU32(utils::BigEndianU32((0x03<<24)|(timediff&0xFFFFFF)));
    stream_->WriteU32(utils::BigEndianU32(((lenght&0xFFFFFF) << 8)|contentType_));

    stream_->WriteU32(0);

    for (size_t i = 0; i < lenght; i += 128)
    {
        size_t dataLength = lenght - i;
        if (dataLength > 128)
        {
            stream_->WriteData(content+i, 128);
            stream_->WriteU8(0xC3);
        }
        else
        {
            stream_->WriteData(content+i, dataLength);
        }
    }
    
    delete[] content;
}

void AMF3::WriteNull (utils::MemoryStream* stream_, void* value_)
{
    stream_->WriteU8(utils::BigEndianU8('\x01'));
}

void AMF3::WriteFalse (utils::MemoryStream* stream_, void* value_)
{
    stream_->WriteU8(utils::BigEndianU8('\x02'));
}

void AMF3::WriteTrue (utils::MemoryStream* stream_, void* value_)
{
    stream_->WriteU8(utils::BigEndianU8('\x03'));
}

void AMF3::WriteBoolean (utils::MemoryStream* stream_, void* value_)
{
    if (value_ == NULL)
    {
        WriteFalse(stream_, NULL);
    }
    else
    {
        WriteTrue(stream_, NULL);
    }
}

void AMF3::WriteIntWithMarker (utils::MemoryStream* stream_, void* value_)
{
    stream_->WriteU8(utils::BigEndianU8('\x04'));
    WriteInt(stream_, value_);
}

void AMF3::WriteInt (utils::MemoryStream* stream_, void* value_)
{
    int value = *((int*)(&value_));

    if (value < 0 || value >= 0x200000)
    {
        stream_->WriteU8(utils::BigEndianU8((((value >> 22) & 0x7f) | 0x80)));
        stream_->WriteU8(utils::BigEndianU8((((value >> 15) & 0x7f) | 0x80)));
        stream_->WriteU8(utils::BigEndianU8((((value >> 8) & 0x7f) | 0x80)));
        stream_->WriteU8(utils::BigEndianU8((value & 0xff)));
    }
    else
    {
        if (value >= 0x4000)
        {
            stream_->WriteU8(utils::BigEndianU8((((value >> 14) & 0x7f) | 0x80)));
        }
        if (value >= 0x80)
        {
            stream_->WriteU8(utils::BigEndianU8((((value >> 7) & 0x7f) | 0x80)));
        }
        stream_->WriteU8(utils::BigEndianU8((value & 0x7f)));
    }
}

void AMF3::WriteDoubleWithMarker (utils::MemoryStream* stream_, void* value_)
{
    stream_->WriteU8(utils::BigEndianU8('\x05'));
    WriteDouble(stream_, value_);
}

void AMF3::WriteDouble (utils::MemoryStream* stream_, void* value_)
{
    double value = *(double*)value_;
    stream_->WriteDouble(utils::BigEndianDouble(value));
}

void AMF3::WriteStringWithMarker (utils::MemoryStream* stream_, void* value_)
{
    stream_->WriteU8(utils::BigEndianU8('\x06'));
    WriteString(stream_, value_);
}

void AMF3::WriteString (utils::MemoryStream* stream_, void* value_)
{
    if (value_)
    {
        const char* str = (const char*)value_;
        int length = strlen(str);

        WriteInt(stream_, (void*)ABNF28BITINTEGER(length));
        if (length > 0)
        {
            stream_->WriteData(str, length);
        }
    }
    else
    {
        WriteInt(stream_, (void*)ABNF28BITINTEGER(0));
    }
}

void AMF3::WriteDateWithMarker (utils::MemoryStream* stream_, void* value_)
{
    stream_->WriteU8(utils::BigEndianU8('\x08'));
    WriteDate(stream_, value_);
}

void AMF3::WriteDate (utils::MemoryStream* stream_, void* value_)
{
    double value = *((uint*)(&value_));
    stream_->WriteU8(utils::BigEndianU8('\x01'));
    WriteDouble(stream_, &value);
}

void AMF3::WriteArrayWithMarker (utils::MemoryStream* stream_, void* value_)
{
    stream_->WriteU8(utils::BigEndianU8('\x09'));
    WriteArray(stream_, value_);
}

void AMF3::WriteArray (utils::MemoryStream* stream_, void* value_)
{
    ds::Array<AMF3_FUNCTION>* arr = (ds::Array<AMF3_FUNCTION>*)value_;
    size_t length = arr->GetSize();
    WriteInt(stream_, (void*)ABNF28BITINTEGER(length));
    stream_->WriteU8(utils::BigEndianU8('\x01'));
    for(uint i = 0; i < length; i++)
    {
        arr->GetElement(i).Call();
    }
}

void AMF3::WriteAssociativeArrayWithMarker (utils::MemoryStream* stream_, void* value_)
{
    stream_->WriteU8(utils::BigEndianU8('\x09'));
    WriteAssociativeArray(stream_, value_);
}

void AMF3::WriteAssociativeArray (utils::MemoryStream* stream_, void* value_)
{
    const ds::Map<char*, AMF3_FUNCTION>* map = (const ds::Map<char*, AMF3_FUNCTION>*)value_;

    stream_->WriteU8(utils::BigEndianU8('\x01'));
    for (ds::Map<char*, AMF3_FUNCTION>::ConstIterator it = map->GetStart(); !map->IsEnd(it); map->Next(&it))
    {
        WriteString(stream_, it->key);
        it->value.Call();
    }
    stream_->WriteU8(utils::BigEndianU8('\x01'));
}

void AMF3::WriteObjectWithMarker (utils::MemoryStream* stream_, void* value_)
{
    stream_->WriteU8(utils::BigEndianU8('\x0A'));
    WriteObject(stream_, value_);
}

void AMF3::WriteObject (utils::MemoryStream* stream_, void* value_)
{
    static char* flexArrayString = "flex.messaging.io.ArrayCollection";
    OutTypedObject* object = (OutTypedObject*)value_;

    uint type = object->GetTypeID();
    if (type == OutTypedObject::TYPE_NONE)
    {
        stream_->WriteU8(utils::BigEndianU8('\x0B'));
        WriteAssociativeArray(stream_, (void*)object->GetMap());
    }
    else if (type == OutTypedObject::TypedObjectType::TYPE_FLEX_MESSAGING_IO_ARRAYCOLLECTION)
    {
        stream_->WriteU8(utils::BigEndianU8('\x07'));
        WriteString(stream_, flexArrayString);
        object->Find("array").Call();
    }
    else
    {
        const ds::Map<const char*, AMF3_FUNCTION>* map;
        WriteInt(stream_, (void*)((object->GetMap()->GetSize() << 4) | 3));
        WriteString(stream_, (void*)object->GetTypeString());

        map = object->GetMap();
        for (ds::Map<const char*, AMF3_FUNCTION>::ConstIterator it = map->GetStart(); !map->IsEnd(it); map->Next(&it))
        {
            WriteString(stream_, (void*)it->key);
        }

        for (ds::Map<const char*, AMF3_FUNCTION>::ConstIterator it = map->GetStart(); !map->IsEnd(it); map->Next(&it))
        {
            it->value.Call();
        }
    }
}

/*
void AMF3::WriteByteArrayWithMarker (utils::MemoryStream* stream_, const char* array_, uint length_)
{
    stream_->WriteU8(utils::BigEndianU8('\x0C'));
    WriteByteArray(stream_, array_, length_);
}

void AMF3::WriteByteArray (utils::MemoryStream* stream_, const char* array_, uint length_)
{
}
*/

void AMF3::Decode (utils::MemoryStream* stream_, std::ostream& out_, Message* message_)
{
    uint8 type;
    stream_->ReadU8(&type);
    switch (type)
    {
        case 0x01:
            out_ << "null";
        break;

        case 0x02:
            out_ << "false";
        break;

        case 0x03:
            out_ << "true";
        break;

        case 0x04:
            out_ << _ReadInt(stream_);
        break;

        case 0x05:
            out_ << std::setprecision(25) << _ReadDouble(stream_);
        break;

        case 0x06:
            out_ << "\"" << _ReadString(stream_, message_).c_str() << "\"";
        break;

        /*
        case 0x07:
            return ReadXML(_stream_, ignore_);
        break;
        */

        case 0x08:
            _ReadDate(stream_, out_, message_);
        break;

        case 0x09:
            _ReadArray(stream_, out_, message_);
        break;

        case 0x0A:
            _ReadObject(stream_, out_, message_);
        break;
        
        /*
        case 0x0B:
            return ReadXMLString(_stream_);
        break;
        */

        case 0x0C:
            _ReadArray(stream_, out_, message_);
        break;

    }
}

int AMF3::_ReadInt (utils::MemoryStream* stream_)
{
    static int signMask = 1 << 28;
    uint32 number;
    uint8 tmp;

    stream_->ReadU8(&tmp);
    if (tmp < 128)
    {
        return tmp;
    }
    
    number = (tmp&0x7F) << 7;
    stream_->ReadU8(&tmp);
    if (tmp < 128)
    {
        number = number | tmp;
    }
    else
    {
        number = (number|(tmp&0x7F)) << 7;
        stream_->ReadU8(&tmp);
        if (tmp < 128)
        {
            number = number | tmp;
        }
        else
        {
            number = (number|(tmp&0x7F)) << 8;
            stream_->ReadU8(&tmp);
            number = number | tmp;
        }
    }

    number = -((signed)(number&signMask))|number;

    return number;
}

double AMF3::_ReadDouble (utils::MemoryStream* stream_)
{
    double value;
    stream_->ReadDouble(&value);
    value = utils::BigEndianDouble(value);

    return value;
}

std::string AMF3::_ReadString (utils::MemoryStream* stream_, Message* message_)
{
    int32 length;
    bool isReference;
    
    length = _ReadInt(stream_);

    isReference = ((length&1) != 0);
    length = length >> 1;

    if (isReference)
    {
        if (length == 0)
        {
            return std::string("");
        }
        std::string string((const char*)stream_->GetCursor(), length);
        stream_->Forward(length);

        // Escape all Json characters...
        boost::replace_all(string, "\\", "\\\\");
        boost::replace_all(string, "\b", "\\\\b");
        boost::replace_all(string, "\f", "\\\\f");
        boost::replace_all(string, "\n", "\\\\n");
        boost::replace_all(string, "\r", "\\\\r");
        boost::replace_all(string, "\t", "\\\\t");
        boost::replace_all(string, "\v", "\\\\v");
        boost::replace_all(string, "\"", "\\\"");

        message_->AddStringReference(string);

        return string;
    }

    return message_->GetStringReference(length);
}

void AMF3::_ReadDate (utils::MemoryStream* stream_, std::ostream& out_, Message* message_)
{
    int32 handle;
    bool isReference;

    handle = _ReadInt(stream_);
    isReference = ((handle&1) != 0);
    handle = handle >> 1;

    if (isReference)
    {
        double value = _ReadDouble(stream_);
        
        std::ostringstream object;
        object << std::setprecision(25) << value;
        out_ << object.str().c_str();

        message_->AddObjectReference(object.str());
        return;
    }

    out_ << std::setprecision(25) << message_->GetObjectReference(handle).c_str();
}

void AMF3::_ReadArray (utils::MemoryStream* stream_, std::ostream& out_, Message* message_)
{
    int32 handle;
    bool isReference;

    handle = _ReadInt(stream_);
    isReference = ((handle&1) != 0);
    handle = handle >> 1;

    if (isReference)
    {
        std::ostringstream object;
        size_t index = message_->GetObjectReferenceSize();
        message_->AddObjectReference(object.str());
        object << "[";
        _ReadString(stream_, message_);

        if (handle >= 1)
        {
            AMF3::Decode(stream_, object, message_);
            for (int i = 1; i < handle; i++)
            {
                object << ",";
                AMF3::Decode(stream_, object, message_);
            }
        }
        
        object << "]";
        message_->UpdateObjectReference(object.str().c_str(), index);
        out_ << object.str().c_str();
        return;
    }
    out_ << message_->GetObjectReference(handle).c_str();
}

void AMF3::_ReadObject (utils::MemoryStream* stream_, std::ostream& out_, Message* message_)
{
    int32 handle;
    bool isReference;

    handle = _ReadInt(stream_);
    isReference = ((handle&1) != 0);
    handle = handle >> 1;

    int32 objReference = handle;

    if (isReference)
    {
        bool isClassReference = ((handle&1) != 0);
        ClassDefinition* cd;
        handle = handle >> 1;
        int classUID = handle;

        if (isClassReference)
        {
            bool externalizable = ((handle&1) != 0);
            handle = handle >> 1;
            bool dynamic = ((handle&1) != 0);
            handle = handle >> 1;

            std::string className = _ReadString(stream_, message_);
            cd = message_->CreateClassDefinition(className, externalizable, dynamic);

            for (int i = 0; i < handle; i++)
            {
                cd->members.push_back(_ReadString(stream_, message_));
            }

            message_->AddClassReference(cd);
        }
        else
        {
            cd = message_->GetClassReference(classUID);
        }

        if (!cd)
        {
            return;
        }

        std::ostringstream object;
        size_t index = message_->GetObjectReferenceSize();
        message_->AddObjectReference(object.str());
        object << "{";

        if (cd->externalizable)
        {
            if (cd->typeID == ClassDefinition::TYPE_DSK)
            {
                _ReadDSK(stream_, object, message_);
            }
            else if (cd->typeID == ClassDefinition::TYPE_DSA)
            {
                _ReadDSA(stream_, object, message_);
            }
            else if (cd->typeID == ClassDefinition::TYPE_FLEX_MESSAGING_IO_ARRAYCOLLECTION)
            {
                object << "\"array\":";
                AMF3::Decode(stream_, object, message_);
            }
            else if (cd->typeID != ClassDefinition::TYPE_UNKNOW)
            {
                int size = 0;

                for (int i = 0; i < 4; i++)
                {
                    uint8 byte;
                    stream_->ReadU8(&byte);
                    size = (size<<8)|byte;
                }
                stream_->Forward(size);

                printf("Attempt to read json data.");
            }
            else
            {
                return;
            }
        }
        else
        {
            std::list<std::string>::iterator it = cd->members.begin();
            
            if (it != cd->members.end())
            {
                object << "\"" << it->c_str() << "\":";
                AMF3::Decode(stream_, object, message_);
                ++it;
            }

            while (it != cd->members.end())
            {
                object << ",\"" << it->c_str() << "\":";
                AMF3::Decode(stream_, object, message_);
                it++;
            }

            if (cd->dynamic)
            {
                bool first = true;
                while (true)
                {
                    std::string key = _ReadString(stream_, message_);
                    if (key.size() == 0)
                    {
                        break;
                    }
                    if (first)
                    {
                        first = false;
                    }
                    else
                    {
                        object << ",";
                    }

                    object << "\"" << key.c_str() << "\":";
                    AMF3::Decode(stream_, object, message_);
                }
                
            }
        }
        object << "}";
        message_->UpdateObjectReference(object.str().c_str(), index);
        
        out_ << object.str().c_str();
        return;
    }

    out_ << message_->GetObjectReference(handle).c_str();
}

void AMF3::_ReadByteArray (utils::MemoryStream* stream_, std::ostream& out_, Message* message_)
{
    int32 handle;
    bool isReference;

    handle = _ReadInt(stream_);
    isReference = ((handle&1) != 0);
    handle = handle >> 1;

    if (isReference)
    {
        std::ostringstream object;

        for (int i = 0; i < handle; i++)
        {
            uint8 byte;
            stream_->ReadU8(&byte);
            object << byte;
        }

        message_->AddObjectReference(object.str());
        return;
    }
    out_ << message_->GetObjectReference(handle).c_str();
}

void AMF3::_ReadDSK (utils::MemoryStream* stream_, std::ostream& out_, Message* message_)
{
    _ReadDSA(stream_, out_, message_);

    std::list<int> flags;
    uint8 flag;

    do
    {
        if (!stream_->ReadU8(&flag))
        {
            return;
        }
        flags.push_back(flag);
    } while ((flag & 0x80) != 0);

    for(std::list<int>::iterator it = flags.begin(); it != flags.end(); it++)
    {
        _ReadRemaining(*it, 0, stream_, message_);
    }
}

void AMF3::_ReadDSA (utils::MemoryStream* stream_, std::ostream& out_, Message* message_)
{
    std::list<int> flags;
    std::list<int>::iterator it;
    bool first = true;
    uint8 flag;

    do
    {
        stream_->ReadU8(&flag);
        flags.push_back(flag);
    } while ((flag & 0x80) != 0);

    it = flags.begin();
    flag = *it;

    if ((flag & 0x01) != 0)
    {
        if (first)
        {
            first = false;
        }
        out_ << "\"body\":";
        AMF3::Decode(stream_, out_, message_);
    }
    if ((flag & 0x02) != 0)
    {
        /*
        if (first)
        {
            first = false;
        }
        else
        {
            out_ << ",";
        }
        out_ << "\"clientId\":";
        */
        std::ostringstream object;
        AMF3::Decode(stream_, object, message_);
    }
    if ((flag & 0x04) != 0)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            out_ << ",";
        }
        out_ << "\"destination\":";
        AMF3::Decode(stream_, out_, message_);
    }
    if ((flag & 0x08) != 0)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            out_ << ",";
        }
        out_ << "\"headers\":";
        AMF3::Decode(stream_, out_, message_);
    }
    if ((flag & 0x10) != 0)
    {
        /*
        if (first)
        {
            first = false;
        }
        else
        {
            out_ << ",";
        }
        out_ << "\"messageId\":";
        */
        std::ostringstream object;
        AMF3::Decode(stream_, object, message_);
    }
    if ((flag & 0x20) != 0)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            out_ << ",";
        }
        out_ << "\"timeStamp\":";
        AMF3::Decode(stream_, out_, message_);
    }
    if ((flag & 0x40) != 0)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            out_ << ",";
        }
        out_ << "\"timeToLive\":";
        AMF3::Decode(stream_, out_, message_);
    }
    _ReadRemaining(*it, 7, stream_, message_);
    ++it;

    if (it != flags.end())
    {
        flag = *it;
        std::ostringstream object;

        if ((flag & 0x01) != 0)
        {
            stream_->Forward(1);
            _ReadByteArray(stream_, object, message_);
        }
        if ((flag & 0x02) != 0)
        {
            stream_->Forward(1);
            _ReadByteArray(stream_, object, message_);
        }

        _ReadRemaining(*it, 2, stream_, message_);
    }
    ++it;

    while (it != flags.end())
    {
        _ReadRemaining(*it, 0, stream_, message_);
        ++it;
    }

    flags.clear();
    do
    {
        stream_->ReadU8(&flag);
        flags.push_back(flag);
    } while ((flag & 0x80) != 0);

    it = flags.begin();
    flag = *it;

    if ((flag & 0x01) != 0)
    {
        /*
        if (first)
        {
            first = false;
        }
        else
        {
            out_ << ",";
        }
        out_ << "\"correlationId\":";
        */
        std::ostringstream object;
        AMF3::Decode(stream_, object, message_);
    }
    if ((flag & 0x02) != 0)
    {
        std::ostringstream object;
        stream_->Forward(1);
        _ReadByteArray(stream_, object, message_);
        //_reference->Insert("correlationIdBytes", ReadByteArray(_stream_, message_));
        //_reference->Insert("correlationId", ByteArrayToID(temp));
    }
    _ReadRemaining(*it, 2, stream_, message_);

    ++it;

    while (it != flags.end())
    {
        _ReadRemaining(*it, 0, stream_, message_);
        ++it;
    }
}

void AMF3::_ReadRemaining (int flag_, int bits_, utils::MemoryStream* stream_, Message* message_)
{
    std::ostringstream object;
    if ((flag_ >> bits_) != 0)
    {
        for (int i = bits_; i < 6; i++)
        {
            if (((flag_ >> i) & 1) != 0)
            {
                AMF3::Decode(stream_, object, message_);
            }
        }
    }
}