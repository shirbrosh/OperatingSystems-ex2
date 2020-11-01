#include "uthreads.h"
#include "Thread.h"
#include "Scheduler.h"
#include <sys/time.h>
#include <stdio.h>
#include <signal.h>

#define MAX_THREAD_NUM 100 /* maximal number of threads */
#define FAIL -1
#define SUCCESS 0
#define EXIT_FAIL 1

#define FAIL_LIB_MSG "thread library error: "
#define FAIL_SYS_MSG "system error: "
#define FAIL_INIT_MSG "size or quantum value is non-positive"
#define FAIL_SPAWN_MSG "threads capacity if full"
#define FAIL_TID_MSG "ID number does not exists"
#define FAIL_PR_MSG "new priority is negative"
#define MAIN_ID_BLOCK_MSG "can not block main thread"
#define ALLOC_MSG "allocation failed"
#define ERROR_BLOCK_MSG "failed to block signals"
#define ERROR_UNBLOCK_MSG "failed to unblock signals"
#define TIMER_ERROR_MSG "setitimer error"
#define SIGACTION_ERROR "sigaction error"
#define SIGEMPTYSET_ERROR "sigemptyset error"
#define SIGADDSET_ERROR "sigaddset error"

struct sigaction sa;
struct itimerval timer;
static Scheduler *scheduler;
sigset_t set;


/**
 * block other signals
 * @return 0 if the action succeed, exit from the program otherwise
 */
int blockSig()
{
    if (sigprocmask(SIG_BLOCK, &set, nullptr) == FAIL)
    {
        std::cerr << FAIL_SYS_MSG << ERROR_BLOCK_MSG << std::endl;
        exit(EXIT_FAIL);
    }
    return SUCCESS;
}

/**
 * unblock the blocked signals
 * @return 0 if the action succeed, exit from the program otherwise
 */
int unblockSig()
{
    if (sigprocmask(SIG_UNBLOCK, &set, nullptr) == FAIL)
    {
        std::cerr << FAIL_SYS_MSG << ERROR_UNBLOCK_MSG << std::endl;
        exit(EXIT_FAIL);
    }
    return SUCCESS;
}

/**
 * set a timer according to the quantum of the next thread that will run
 * @param quantum - the quantum of the next thread that will run.
 */
void setTimer(int quantum)
{
    timer.it_value.tv_sec = quantum / 1000000;        // first time interval, seconds part
    timer.it_value.tv_usec = quantum % 1000000;        // first time interval, microseconds part

    timer.it_interval.tv_sec = quantum / 1000000;    // following time intervals, seconds part
    timer.it_interval.tv_usec = quantum % 1000000;    // following time intervals, microseconds part
    if (setitimer(ITIMER_VIRTUAL, &timer, NULL) == FAIL)
    {
        std::cerr << FAIL_SYS_MSG << TIMER_ERROR_MSG << std::endl;
        exit(EXIT_FAIL);
    }
}

/*~~~~~~~~~ handle threads switch ~~~~~~~~~*/

/**
 * @return the next thread that should run
 */
Thread *getNextThread(Thread *nextToRun)
{
    nextToRun = scheduler->getReadyThreadsQueue()->front();
    scheduler->removeFromReadyThreadsQueue(nextToRun->getID());
    nextToRun->setState(RUNNING);
    scheduler->setRunningThread(nextToRun);
    return nextToRun;
}

/**
 * set the amount of quantums of the given thread, and the total amount of quantums
 * @param curRunning - the running thread
 */
void setQuantums(Thread *curRunning)
{
    scheduler->setTotalQuantums();
    curRunning->setCountQuantums();
}

/**
 * the handler function of the virtual timer.
 * switch between the thread that is currently running and the thread that is first on the queue.
 */
void switchThreads(int sigNum = 0)
{
    blockSig();
    Thread *curRunning = scheduler->getRunningThread();

    //clear the deleted vector
    scheduler->getRecentlyDeleted().clear();

    //check if the queue is empty
    if (scheduler->getReadyThreadsQueue()->empty())
    {
        setTimer(curRunning->getQuantum());
        scheduler->setTotalQuantums();
        curRunning->setCountQuantums();
        unblockSig();
        return;
    }

    // if the running thread is terminating itself:
    if (curRunning->getState() == TERMINATED)
    {
        scheduler->removeFromThreadsMap(curRunning->getID());
        scheduler->addRecentlyDeletedVec(curRunning);
    }

    else        //in case the thread is running or blocked
    {
        int ret_val = sigsetjmp(curRunning->env, 1);
        if (ret_val == 1)
        {
            unblockSig();
            return;
        }
        if (curRunning->getState() == RUNNING)
        {
            curRunning->setState(READY);
            scheduler->addReadyThreadsQueue(&curRunning);
        }
    }

    curRunning = getNextThread(curRunning);
    setTimer(curRunning->getQuantum());
    setQuantums(curRunning);
    unblockSig();
    siglongjmp(curRunning->env, 1);
}

/**
 * check signals errors
 */
void handleSignals()
{
    if (sigaction(SIGVTALRM, &sa, NULL) == FAIL)
    {
        std::cerr << FAIL_SYS_MSG << SIGACTION_ERROR << std::endl;
        exit(1);
    }
    if (sigemptyset(&sa.sa_mask) == FAIL)
    {
        std::cerr << FAIL_SYS_MSG << SIGEMPTYSET_ERROR << std::endl;
        exit(1);
    }
    if (sigemptyset(&set) == FAIL)
    {
        std::cerr << FAIL_SYS_MSG << SIGEMPTYSET_ERROR << std::endl;
        exit(1);
    }
    if (sigaddset(&set, SIGVTALRM) == FAIL)
    {
        std::cerr << FAIL_SYS_MSG << SIGADDSET_ERROR << std::endl;
        exit(1);
    }
}

/**
 * define sa_handler function as switchThreads() and deals whit other signals
 */
void initSignalSet()
{
    sa.sa_handler = &switchThreads;
    sa.sa_flags = 0;
    handleSignals();
}

/*~~~~~~~~~ argument check ~~~~~~~~~*/

/**
 * check if a number in the given list is smaller or equal to zero
 * @param quantum_usecs - the list to check in
 * @param size - the size of the list
 * @return true if there is a number in the list that is smaller or equal to zero
 */
bool isNonPositive(int *quantum_usecs, int size)
{
    for (int i = 0; i < size; ++i)
    {
        if (quantum_usecs[i] <= 0)
        {
            return true;
        }
    }
    return false;
}

/*~~~~~~~~~ uthreads library functions ~~~~~~~~~*/

/**
 * This function initializes the thread library.
 * You may assume that this function is called before any other thread library
 * function, and that it is called exactly once. The input to the function is
 * an array of the length of a quantum in micro-seconds for each priority.
 * It is an error to call this function with an array containing non-positive integer.
 * @param quantum_usecs -an array of the length of a quantum in micro-seconds for each priority
 * @param size- is the size of the array.
 * @return On success, return 0. On failure, return -1.
 */
int uthread_init(int *quantum_usecs, int size)
{
    blockSig();
    if (size <= 0 || isNonPositive(quantum_usecs, size))
    {
        std::cerr << FAIL_LIB_MSG << FAIL_INIT_MSG << std::endl;
        unblockSig();
        return FAIL;
    }
    initSignalSet();
    scheduler = new Scheduler(quantum_usecs, size);
    uthread_spawn(nullptr, MAIN_THREAD);
    scheduler->setRunningThread((*scheduler->getThreadsMap())[MAIN_THREAD]);
    setTimer((*scheduler->getThreadsMap())[MAIN_THREAD]->getQuantum());
    unblockSig();
    return SUCCESS;
}


/**
 * This function creates a new thread, whose entry point is the
 * function f with the signature void f(void). The thread is added to the end
 * of the READY threads list. The uthread_spawn function should fail if it
 * would cause the number of concurrent threads to exceed the limit
 * (MAX_THREAD_NUM). Each thread should be allocated with a stack of size
 * STACK_SIZE bytes.
 * @param f- the entry point of the new thread
 * @param priority- The priority of the new thread.
 * @return-On success, return the ID of the created thread.
 * On failure, return -1.
 */
int uthread_spawn(void (*f)(void), int priority)
{
    blockSig();
    int newID = scheduler->getAvailableID();
    if (newID == FAIL)
    {
        std::cerr << FAIL_LIB_MSG << FAIL_SPAWN_MSG << std::endl;
        unblockSig();
        return FAIL;
    }
    Thread *newThread;
    if (newID == MAIN_THREAD)   // in case adding main Thread
    {
        newThread = new Thread(newID, scheduler->getQuantum_usecs()[priority], priority, f, RUNNING,
                               1);
        if (newThread->getStack() == nullptr)
        {
            std::cerr << ALLOC_MSG << std::endl;
            unblockSig();
            exit(EXIT_FAIL);
        }
    }
    else
    {
        newThread = new Thread(newID, scheduler->getQuantum_usecs()[priority], priority, f);
        if (newThread->getStack() == nullptr)
        {
            std::cerr << ALLOC_MSG << std::endl;
            unblockSig();
            exit(EXIT_FAIL);
        }
        scheduler->addReadyThreadsQueue(&newThread);
    }
    scheduler->addThreadsMap(newThread);
    unblockSig();
    return newID;
}


/**
 * This function changes the priority of the thread with ID tid. If this is the current running
 * thread, the effect should take place only the
 * next time the thread gets scheduled.
 * @param tid - thread ID number
 * @param priority - the new priority
 * @return On success, return 0. On failure, return -1
 */
int uthread_change_priority(int tid, int priority)
{
    if (!scheduler->containsKeyThreadsMap(tid))
    {
        std::cerr << FAIL_LIB_MSG << FAIL_TID_MSG << std::endl;
        return FAIL;
    }
    if (priority < 0)
    {
        std::cerr << FAIL_LIB_MSG << FAIL_PR_MSG << std::endl;
        return FAIL;
    }
    (*scheduler->getThreadsMap())[tid]->setPriority(priority);
    return SUCCESS;
}

/**
 * erase all the threads in ThreadsMap
 */
void eraseAllThreads()
{
    auto iter = scheduler->getThreadsMap()->begin();
    while (iter != scheduler->getThreadsMap()->end())
    {
        delete (*iter).second;
        (*iter).second = nullptr;
        ++iter;
    }
}

/**
 * terminate the main thread after erasing all the other threads and releasing resources
 * @return
 */
void terminateMainThread()
{
    scheduler->getReadyThreadsQueue()->clear();
    scheduler->getBlockedMap()->clear();
    scheduler->getRecentlyDeleted().clear();
    eraseAllThreads();
    scheduler->getThreadsMap()->clear();
    sigemptyset(&set);
}

/**
 * This function terminates the thread with ID tid and deletes
 * it from all relevant control structures. All the resources allocated by
 * the library for this thread should be released. If no thread with ID tid
 * exists it is considered an error. Terminating the main thread
 * (tid == 0) will result in the termination of the entire process using
 * exit(0) [after releasing the assigned library memory].
 * @param tid - ID thread to terminate
 * @return The function returns 0 if the thread was successfully
 * terminated and -1 otherwise. If a thread terminates itself or the main
 * thread is terminated, the function does not return.
 */
int uthread_terminate(int tid)
{
    blockSig();
    if (!scheduler->containsKeyThreadsMap(tid))
    {
        std::cerr << FAIL_LIB_MSG << FAIL_TID_MSG << std::endl;
        unblockSig();
        return FAIL;
    }
    if (tid != MAIN_THREAD)
    {
        Thread *toDelete = (*scheduler->getThreadsMap())[tid];
        switch ((*scheduler->getThreadsMap())[tid]->getState())
        {
            case RUNNING:
                (*scheduler->getThreadsMap())[tid]->setState(TERMINATED);
                switchThreads();
                break;
            case BLOCKED:
                scheduler->removeFromBlockedThreadsMap(tid);
                scheduler->removeFromThreadsMap(tid);
                break;
            case READY:
                scheduler->removeFromReadyThreadsQueue(tid);
                scheduler->removeFromThreadsMap(tid);
                break;
        }
        delete toDelete;
        toDelete = nullptr;
    }
    else
    {
        terminateMainThread();
        unblockSig();
        exit(SUCCESS);
    }
    unblockSig();
    return SUCCESS;
}

/**
 * block the thread with the given tid
 * @param tid - ID of the thread
 */
void blockThread(int tid)
{
    scheduler->addBlockedThreadsMap((*scheduler->getThreadsMap())[tid]);
    (*scheduler->getThreadsMap())[tid]->setState(BLOCKED);
}

/**
 * his function blocks the thread with ID tid. The thread may
 * be resumed later using uthread_resume. If no thread with ID tid exists it
 * is considered as an error. In addition, it is an error to try blocking the
 * main thread (tid == 0). If a thread blocks itself, a scheduling decision
 * should be made. Blocking a thread in BLOCKED state has no
 * effect and is not considered an error.
 * @param tid -thread ID
 * @return On success, return 0. On failure, return -1.
 */
int uthread_block(int tid)
{
    blockSig();
    if (!scheduler->containsKeyThreadsMap(tid))
    {
        std::cerr << FAIL_LIB_MSG << FAIL_TID_MSG << std::endl;
        unblockSig();
        return FAIL;
    }
    if (tid == MAIN_THREAD)
    {
        std::cerr << FAIL_LIB_MSG << MAIN_ID_BLOCK_MSG << std::endl;
        unblockSig();
        return FAIL;
    }
    if ((*scheduler->getThreadsMap())[tid]->getState() == RUNNING)
    {
        blockThread(tid);
        switchThreads();
    }
    else if ((*scheduler->getThreadsMap())[tid]->getState() == READY)
    {
        scheduler->removeFromReadyThreadsQueue(tid);
        blockThread(tid);
    }
    unblockSig();
    return SUCCESS;
}

/**
 * return thread to the readyQueue and set it's state to READY
 * @param tid - ID of thread to resume
 */
void resumeThread(int tid)
{
    (*scheduler->getThreadsMap())[tid]->setState(READY);
    scheduler->removeFromBlockedThreadsMap(tid);
    scheduler->addReadyThreadsQueue(&(*scheduler->getThreadsMap())[tid]);
}

/**
 * This function resumes a blocked thread with ID tid and moves
 * it to the READY state. Resuming a thread in a RUNNING or READY state
 * has no effect and is not considered as an error. If no thread with
 * ID tid exists it is considered an error.
 * @param tid - thread ID
 * @return On success, return 0. On failure, return -1.
 */
int uthread_resume(int tid)
{
    blockSig();
    if (!scheduler->containsKeyThreadsMap(tid))
    {
        std::cerr << FAIL_LIB_MSG << FAIL_TID_MSG << std::endl;
        unblockSig();
        return FAIL;
    }
    if ((*scheduler->getThreadsMap())[tid]->getState() == BLOCKED)
    {
        resumeThread(tid);
    }
    unblockSig();
    return SUCCESS;
}

/**
 * This function returns the thread ID of the calling thread.
 * @return -  The ID of the calling thread.
 */
int uthread_get_tid()
{
    return scheduler->getRunningThread()->getID();
}


/**
 * This function returns the total number of quantums since
 * the library was initialized, including the current quantum.
 * Right after the call to uthread_init, the value should be 1.
 * Each time a new quantum starts, regardless of the reason, this number
 * should be increased by 1.
 * @return- The total number of quantums.
 */
int uthread_get_total_quantums()
{
    return scheduler->getTotalQuantums();
}


/**
 * This function returns the number of quantums the thread with
 * ID tid was in RUNNING state. On the first time a thread runs, the function
 * should return 1. Every additional quantum that the thread starts should
 * increase this value by 1 (so if the thread with ID tid is in RUNNING state
 * when this function is called, include also the current quantum). If no
 * thread with ID tid exists it is considered an error.
 * @param tid - thread ID
 * @return On success, return the number of quantums of the thread with ID tid.
 * 			     On failure, return -1.
 */
int uthread_get_quantums(int tid)
{
    blockSig();
    if (!scheduler->containsKeyThreadsMap(tid))
    {
        std::cerr << FAIL_LIB_MSG << FAIL_TID_MSG << std::endl;
        unblockSig();
        return FAIL;
    }
    unblockSig();
    return (*scheduler->getThreadsMap())[tid]->getCountQuantums();
}



