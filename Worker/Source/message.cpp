#include "message.h"

Message::Message()
    : position(0), 
      size(0), 
      m_classDefinitionAllocator(20), 
      m_stringReference(1000),
      m_objectReference(1000)
{
    message = new uint8[256*256*256];
};

Message::~Message()
{
    delete[] message;
}

void Message::Clear ()
{
    position = 0;
    for (size_t index = 0; index < m_classesDefinitions.size(); index++)
    {
        m_classDefinitionAllocator.Release(m_classesDefinitions[index]);
    }
    
    m_classesDefinitions.clear();
    m_objectReference.clear();
    m_stringReference.clear();
}

void Message::AddStringReference (std::string value_)
{
    m_stringReference.push_back(value_);
}

std::string Message::GetStringReference (uint32 index_)
{
    if (index_ < m_stringReference.size())
    {
        return m_stringReference.at(index_);
    }
    return std::string("");
}

void Message::AddObjectReference (std::string obj_)
{
     m_objectReference.push_back(obj_);
}

void Message::UpdateObjectReference (std::string obj_, uint32 index_)
{
    if (index_ < m_objectReference.size())
    {
        m_objectReference[index_] = obj_;
    }
}

std::string Message::GetObjectReference (uint32 index_)
{
    if (index_ < m_objectReference.size())
    {
        return m_objectReference.at(index_);
    }
    return std::string("");
}

size_t Message::GetObjectReferenceSize ()
{
    return m_objectReference.size();
}

void Message::AddClassReference (ClassDefinition* obj_, int index_)
{
    m_classesDefinitions.push_back(obj_);
}

ClassDefinition* Message::GetClassReference (int32 index_)
{
    return m_classesDefinitions.at(index_);
}
