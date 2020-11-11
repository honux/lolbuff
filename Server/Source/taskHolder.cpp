#include "taskHolder.h"
#include "task.h"
#include "connection.h"
#include <algorithm>

TaskHolder::TaskHolder()
    :m_taskAllocator(100, 1.5)
{
}

Task* TaskHolder::CreateTask (std::string destination_, std::string operation_, Connection* connection_)
{
    Task* task = new(m_taskAllocator) Task(destination_, operation_, connection_, true);
    m_taskList.push_back(task);
    return task;
}

void TaskHolder::FreeTask (Task* task_)
{
    m_taskList.remove(task_);

    m_taskAllocator.Release(task_);
}

Task* TaskHolder::Find (uint32 taskID_)
{
    TaskSelector selector(taskID_);
    std::list<Task*>::iterator it = std::find_if(m_taskList.begin(), m_taskList.end(), selector);

    if (it != m_taskList.end())
    {
        return (*it);
    }
    return NULL;
}

TaskHolder& TaskHolder::GetInstance()
{
    static TaskHolder instance;
    return instance;
}