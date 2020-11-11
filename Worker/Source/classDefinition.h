#ifndef _CLASS_DEFINITION_H_
#define _CLASS_DEFINITION_H_

#include "types.h"
#include <list>
#include <string>

struct ClassDefinition
{
    ClassDefinition (std::string type_, bool externalizable_, bool dynamic_)
        : externalizable(externalizable_), 
          dynamic(dynamic_),
          type(type_)
    {
        if (type.size() == 0)
        {
            typeID = ClassDefinition::TYPE_UNKNOW;
        }
        else if (type.compare("DSK") == 0)
        {
            typeID = ClassDefinition::TYPE_DSK;
        }
        else if (type.compare("DSA") == 0)
        {
            typeID = ClassDefinition::TYPE_DSA;
        }
        else if (type.compare("flex.messaging.io.ArrayCollection") == 0)
        {
            typeID = ClassDefinition::TYPE_FLEX_MESSAGING_IO_ARRAYCOLLECTION;
        }
        else if (type.compare("com.riotgames.platform.systemstate.ClientSystemStatesNotification") == 0)
        {
            typeID = ClassDefinition::TYPE_COM_RIOTGAMES_PLATFORM_SYSTEMSTATE_CLIENTSYSTEMSTATESNOTIFICATION;
        }
        else if (type.compare("com.riotgames.platform.broadcast.BroadcastNotification") == 0)
        {
            typeID = ClassDefinition::TYPE_COM_RIOTGAMES_PLATFORM_BROADCAST_BROADCASTNOTIFICATION;
        }
        else
        {
            typeID = ClassDefinition::TYPE_UNKNOW;
        }
    }

    ~ClassDefinition ()
    {
    }

    enum ClassDefinitionType
    {
        TYPE_DSK                                = 0,
        TYPE_DSA                                = 1,
        TYPE_FLEX_MESSAGING_IO_ARRAYCOLLECTION  = 2,
        TYPE_COM_RIOTGAMES_PLATFORM_SYSTEMSTATE_CLIENTSYSTEMSTATESNOTIFICATION  = 3,
        TYPE_COM_RIOTGAMES_PLATFORM_BROADCAST_BROADCASTNOTIFICATION = 4,

        TYPE_UNKNOW                             = 5
    };
    
    uint typeID;
    std::string type;
    bool externalizable;
    bool dynamic;
    std::list<std::string> members;
};

#endif