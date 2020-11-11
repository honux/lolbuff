#ifndef __OUTTYPED_OBJECT_H__
#define __OUTTYPED_OBJECT_H__

#include "amf3.h"

class OutTypedObject
{
public:
    enum TypedObjectType
    {
        // No defined type
        TYPE_NONE                               = 0,
        // flex.messaging.io.ArrayCollection
        TYPE_FLEX_MESSAGING_IO_ARRAYCOLLECTION  = 1,
        // Other type, explicit on m_typeExtended
        TYPE_OTHER                              = 3
    };

    inline OutTypedObject ();
    inline OutTypedObject (uint type_);
    inline ~OutTypedObject ();

    AMF3_FUNCTION Find (const char* key_)
    {
        AMF3_FUNCTION ptr = NULL;

        m_map.Find(key_, &ptr);
        
        return ptr;
    }

    void Insert (const char* key_, AMF3_FUNCTION value_)
    {
        m_map.Insert(key_, value_);
    }

    void Remove (const char* key_)
    {
        m_map.Remove(key_);
    }
    
    void SetType (uint type_, const char* typeString_)
    {
        m_typeID = type_;
        m_typeString = typeString_;
    }

    uint GetTypeID () const
    {
        return m_typeID;
    }

    const char* GetTypeString () const
    {
        return m_typeString;
    }

    const ds::Map<const char*, AMF3_FUNCTION>* GetMap ()
    {
        return &m_map;
    }

    void Clear ()
    {
        m_map.Clear();
    }

private:
    uint m_typeID;
    const char* m_typeString;
    ds::Map<const char*, AMF3_FUNCTION> m_map;
};

inline OutTypedObject::OutTypedObject ()
    :m_map(10),
     m_typeID(TypedObjectType::TYPE_NONE),
     m_typeString(NULL)
{
}

inline OutTypedObject::OutTypedObject (uint type_)
    :m_map(10),
     m_typeID(TypedObjectType::TYPE_NONE),
     m_typeString(NULL)
{
}

inline OutTypedObject::~OutTypedObject ()
{
}
#endif