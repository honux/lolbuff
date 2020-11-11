#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include "types.h"
#include "classDefinition.h"
#include "map.h"
#include "allocator.h"
#include <vector>

namespace utils
{
    class MemoryStream;
};

class Client;

class Message
{
    friend class Client;
public:
    Message();
    ~Message();
    
    void Clear ();

    void AddStringReference (std::string value_);
    std::string GetStringReference (uint32 index_);

    void AddObjectReference (std::string obj_);
    void UpdateObjectReference (std::string obj_, uint32 index_);
    std::string GetObjectReference (uint32 index_);
    size_t GetObjectReferenceSize ();

    void AddClassReference (ClassDefinition* obj_, int index_ = -1);
    ClassDefinition* GetClassReference (int32 index_);

    ClassDefinition* CreateClassDefinition (std::string type_, bool externalizable_, bool dynamic_)
    {
        return new(m_classDefinitionAllocator) ClassDefinition(type_, externalizable_, dynamic_);
    }

private:
    uint8* message;
    size_t position;
    size_t size;
    uint type;

    std::vector<std::string> m_stringReference;
    std::vector<std::string> m_objectReference;
    std::vector<ClassDefinition*> m_classesDefinitions;

    utils::MemoryPool<ClassDefinition> m_classDefinitionAllocator;
};

#endif