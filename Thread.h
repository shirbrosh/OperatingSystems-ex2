#include <iostream>
#include <setjmp.h>
#include <signal.h>

#ifndef THREAD_H
#define THREAD_H
//TODO change to 4096
#define STACK_SIZE 16384 /* stack size per thread (in bytes) */

typedef enum States
{
    RUNNING, BLOCKED, READY, TERMINATED
} States;

class Thread
{
private:
    int _ID;
    int _quantum;
    int _priority;
    void (*_func)(void);
    States _state;
    int _countQuantums;
    char *_stack;


public:
    sigjmp_buf env;

/**
 * Thread constructor
 */
    Thread(int ID, int quantum, int priority, void(*func)(void), States state = READY, int
    countQuantums = 0);

/**
 * Thread destructor
 */
    ~Thread();

/**
 * @return The ID of the Thread
 */
    int getID() const;

/**
 * @return The state of the Thread
 */
    States getState() const;

/**
 * @return The quantum of the Thread
 */
    int getQuantum() const;

/**
 * change the priority of the thread
 * @param newPriority - new priority
 */
    void setPriority(int newPriority);

/**
 * changed the state of the thread
 * @param state - new state
 */
    void setState(States state);

/**
 * @return The stack of the Thread
 */
    char *getStack();

/**
 * @return The amount of quantum the thread runs
 */
    int getCountQuantums() const;

/**
 * increase the amount of quantum that the thread runs by 1
 */
    void setCountQuantums();

};


#endif
