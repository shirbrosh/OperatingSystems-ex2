#include "Thread.h"

/*~~~~~~~~~ translate variable's address ~~~~~~~~~*/
#ifdef __x86_64__
/* ~~~~~~~~ code for 64 bit Intel arch ~~~~~~~~*/

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%fs:0x30,%0\n"
                 "rol    $0x11,%0\n"
    : "=g" (ret)
    : "0" (addr));
    return ret;
}

#else
/* ~~~~~~~~ code for 32 bit Intel arch ~~~~~~~~*/

typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5


/**
 * A translation is required when using an address of a variable.
   Use this as a black box in your code.
 */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%gs:0x18,%0\n"
        "rol    $0x9,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
    return ret;
}

#endif


/**
 * Thread constructor
 */
Thread::Thread(int ID, int quantum, int priority, void(*func)(void), States state, int
countQuantums) : _ID(ID),
                 _quantum(quantum), _priority(priority), _func(func), _state(state),
                 _countQuantums(countQuantums)
{
    _stack = new(std::nothrow) char[STACK_SIZE];
    if (_stack != nullptr)
    {
        address_t sp, pc;
        sp = (address_t) _stack + STACK_SIZE - sizeof(address_t);
        pc = (address_t) _func;
        sigsetjmp(env, 1);
        (env->__jmpbuf)[JB_SP] = translate_address(sp);
        (env->__jmpbuf)[JB_PC] = translate_address(pc);
        sigemptyset(&env->__saved_mask);
    }
}


/**
 * Thread destructor
 */
Thread::~Thread()
{
    delete[] _stack;
    _stack = nullptr;
}

/**
 * @return The ID of the Thread
 */
int Thread::getID() const
{
    return this->_ID;
}

/**
 * @return The quantum of the Thread
 */
int Thread::getQuantum() const
{
    return this->_quantum;
}

/**
 * change the priority of the thread
 * @param newPriority - new priority
 */
void Thread::setPriority(int newPriority)
{
    this->_priority = newPriority;
}

/**
 * @return The state of the Thread
 */
States Thread::getState() const
{
    return _state;
}

/**
 * @return The stack of the Thread
 */
char *Thread::getStack()
{
    return _stack;
}

/**
 * changed the state of the thread
 * @param state - new state
 */
void Thread::setState(States state)
{
    this->_state = state;
}

/**
 * @return The amount of quantum the thread runs
 */
int Thread::getCountQuantums() const
{
    return _countQuantums;
}

/**
 * increase the amount of quantum that the thread runs by 1
 */
void Thread::setCountQuantums()
{
    this->_countQuantums++;
}





