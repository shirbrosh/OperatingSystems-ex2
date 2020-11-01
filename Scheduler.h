#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <map>
#include <deque>
#include <vector>
#include "Thread.h"

#define MAIN_THREAD 0
#define MAX_THREAD_NUM 100
#define FAIL -1
#define INIT_TOTAL_QUANTUMS 1

class Scheduler
{
public:

/**
 * Scheduler constructor
 * @param quantum_usecs - quantums list
 * @param size - the size of the given list
 */
    explicit Scheduler(int *quantum_usecs, int size);

/**
 * Scheduler destructor
 */
    ~Scheduler();

/**
 * check if _threadsMap contains a given key
 * @param key the key to check
 * @return true if the key is in the map, false otherwise
 */
    bool containsKeyThreadsMap(int key) const;

/**
 * check if there is a number between 0 to 99 that is not using as thread ID
 * @return the available number is there is one, -1 otherwise
 */
    int getAvailableID() const;

/**
 * @return the quantum list
 */
    int *getQuantum_usecs() const;

/**
 * @return a pointer to _threadsMap
 */
    std::map<int, Thread *> *getThreadsMap();

/**
 * add new thread to _threadsMap
 * @param newThread - the thread to add
 */
    void addThreadsMap(Thread *newThread);

/**
 * add new thread to _readyThreadsQueue
 * @param newThread - the thread to add
 */
    void addReadyThreadsQueue(Thread **newThread);

/**
 * add new thread to _blockedThreadsMap
 * @param newThread - the thread to add
 */
    void addBlockedThreadsMap(Thread *newThread);

/**
 * remove a thread from _readyThreadsQueue
 * @param tid - the ID of the thread that need to be removed
 */
    void removeFromReadyThreadsQueue(int tid);

/**
 * remove a thread from _blockedThreadsMap
 * @param tid - the ID of the thread that need to be removed
 */
    void removeFromBlockedThreadsMap(int tid);

/**
 * remove a thread from _threadsMap
 * @param tid - the ID of the thread that need to be removed
 */
    void removeFromThreadsMap(int tid);

/**
 * @return _runningThread map
 */
    Thread *getRunningThread();

/**
 * chane the thread that is currently running
 * @param newThread - new running thread
 */
    void setRunningThread(Thread *newThread);

/**
 * @return _readyThreadsQueue queue - the queue contains all threads with state READY
 */
    std::deque<Thread *> *getReadyThreadsQueue();

/**
 * @return the total amount of Quantums
 */
    int getTotalQuantums() const;

/**
 * increase the amount of the total quantums by 1
 */
    void setTotalQuantums();

/**
 * @return _blockedThreadsMap map - the map contains all threads with state BLOCKED
 */
    std::map<int, Thread *> *getBlockedMap();

/**
* @return _recentlyDeleted vector - the vector contains all threads that terminated themselves
* and had'nt been deleted yet
*/
    std::vector<Thread *> getRecentlyDeleted();

/**
 * add new thread to _recentlyDeleted
 * @param newThread - the thread to add
 */
    void addRecentlyDeletedVec(Thread *newThread);

private:
    int _quantum_usecs_size;
    int *_quantum_usecs;
    int _totalQuantums;
    Thread *_runningThread;
    std::map<int, Thread *> _threadsMap;
    std::deque<Thread *> _readyThreadsQueue;
    std::map<int, Thread *> _blockedThreadsMap;
    std::vector<Thread *> _recentlyDeleted;


};

#endif
