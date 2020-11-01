#include "Scheduler.h"


/**
 * Scheduler constructor
 * @param quantum_usecs - quantums list
 * @param size - the size of the given list
 */
Scheduler::Scheduler(int *quantum_usecs, int size) : _quantum_usecs_size(size),
                                                     _quantum_usecs(quantum_usecs),
                                                     _totalQuantums(INIT_TOTAL_QUANTUMS),
                                                     _runningThread(nullptr)
{}

/**
 * check if _threadsMap contains a given key
 * @param key the key to check
 * @return true if the key is in the map, false otherwise
 */
bool Scheduler::containsKeyThreadsMap(int key) const
{
    return _threadsMap.find(key) != _threadsMap.end();
}

/**
 * check if there is a number between 0 to 99 that is not using as thread ID
 * @return the available number is there is one, -1 otherwise
 */
int Scheduler::getAvailableID() const
{
    for (int i = 0; i < MAX_THREAD_NUM; ++i)
    {
        if (!containsKeyThreadsMap(i))
        {
            return i;
        }
    }
    return FAIL;
}

/**
 * @return the quantum list
 */
int *Scheduler::getQuantum_usecs() const
{
    return this->_quantum_usecs;
}

/**
 * add new thread to _threadsMap
 * @param newThread - the tread to add
 */
void Scheduler::addThreadsMap(Thread *newThread)
{
    this->_threadsMap[newThread->getID()] = newThread;
}

/**
 * add new thread to _readyThreadsQueue
 * @param newThread - the tread to add
 */
void Scheduler::addReadyThreadsQueue(Thread **newThread)
{
    this->_readyThreadsQueue.push_back((*newThread));
}

/**
 * add new thread to _blockedThreadsMap
 * @param newThread - the tread to add
 */
void Scheduler::addBlockedThreadsMap(Thread *newThread)
{
    this->_blockedThreadsMap[newThread->getID()] = newThread;
}

/**
 * @return a pointer to _threadsMap
 */
std::map<int, Thread *> *Scheduler::getThreadsMap()
{
    return &_threadsMap;
}

/**
 * remove a thread from _readyThreadsQueue
 * @param tid - the ID of the thread that need to be removed
 */
void Scheduler::removeFromReadyThreadsQueue(int tid)
{
    auto iter = _readyThreadsQueue.begin();
    while (iter != _readyThreadsQueue.end())
    {
        if ((*iter)->getID() == tid)
        {
            _readyThreadsQueue.erase(iter);
            return;
        }
        ++iter;
    }
}

/**
 * remove a thread from _blockedThreadsMap
 * @param tid - the ID of the thread that need to be removed
 */
void Scheduler::removeFromBlockedThreadsMap(int tid)
{
    _blockedThreadsMap.erase(tid);
}

/**
 * remove a thread from _threadsMap
 * @param tid - the ID of the thread that need to be removed
 */
void Scheduler::removeFromThreadsMap(int tid)
{
    _threadsMap.erase(tid);
}

/**
 * @return _runningThread map
 */
Thread *Scheduler::getRunningThread()
{
    return this->_runningThread;
}

/**
 * @return _readyThreadsQueue queue - the queue contains all threads with state READY
 */
std::deque<Thread *> *Scheduler::getReadyThreadsQueue()
{
    return &(this->_readyThreadsQueue);
}

/**
 * @return the total amount of Quantums
 */
int Scheduler::getTotalQuantums() const
{
    return this->_totalQuantums;
}

/**
 * increase the amount of the total quantums by 1
 */
void Scheduler::setTotalQuantums()
{
    this->_totalQuantums++;
}

/**
 * chane the thread that is currently running
 * @param newThread - new running thread
 */
void Scheduler::setRunningThread(Thread *newThread)
{
    this->_runningThread = newThread;
}

/**
 * Scheduler destructor
 */
Scheduler::~Scheduler()
{
    auto iter = getThreadsMap()->begin();
    while (iter != getThreadsMap()->end())
    {
        delete (*iter).second;
        (*iter).second = nullptr;
        ++iter;
    }
    _readyThreadsQueue.clear();
    _blockedThreadsMap.clear();
    _recentlyDeleted.clear();
    _threadsMap.clear();
}

/**
 * @return _blockedThreadsMap map - the map contains all threads with state BLOCKED
 */
std::map<int, Thread *> *Scheduler::getBlockedMap()
{
    return &_blockedThreadsMap;
}

/**
* @return _recentlyDeleted vector - the vector contains all threads that terminated themselves
 * and had'nt been deleted yet
*/
std::vector<Thread *> Scheduler::getRecentlyDeleted()
{
    return _recentlyDeleted;
}


/**
 * add new thread to _recentlyDeleted
 * @param newThread - the thread to add
 */
void Scheduler::addRecentlyDeletedVec(Thread *newThread)
{
    _recentlyDeleted.push_back(newThread);
}








