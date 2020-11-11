#ifndef _TASKHOLDER_H_
#define _TASKHOLDER_H_

#include "allocator.h"
#include "requestTypes.h"
#include <string>
#include <list>

class Task;
struct bufferevent;
class Connection;

class TaskHolder
{
public:
    Task* CreateTask (std::string destination_, std::string operation_, Connection* connection_);

    void FreeTask (Task* task_);

    Task* Find (uint32 taskID_);

    static TaskHolder& GetInstance();
private:
    TaskHolder();

    utils::MemoryPool<Task> m_taskAllocator;
    std::list<Task*> m_taskList;
};

#endif