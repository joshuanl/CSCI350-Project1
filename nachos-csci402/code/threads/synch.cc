// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"
#include <vector>
#include <iostream>

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!
Lock::Lock(char* debugName) {
    name = debugName;
    owner = NULL;
    BUSY = false;

}
Lock::~Lock() {
    //delete name;
    delete owner;
}
void Lock::Acquire() {
    IntStatus old = interrupt->SetLevel(IntOff);    //first set interrupts off
    if(owner != NULL && owner == currentThread){
       (void) interrupt->SetLevel(old);;
       return;
    }//end of if current thread is already lock owner
    if(!BUSY){                                  //if not busy then available
        BUSY = true;
        owner = currentThread;
    }
    else{                                     //lock is acquired by someone else so put currentthread to sleep
        lockWaitQueue.push_back(currentThread);
        currentThread->Sleep();
    }
    (void) interrupt->SetLevel(old);
    return;

}
void Lock::Release() {
    IntStatus old = interrupt->SetLevel(IntOff);    //first set interrupts off
    if(currentThread != owner){
        std::cout << " >> Error!  Trying to release a lock.\n >> You are not the lock owner!" << std::endl;
        std::cout << " >> Name of Thread: " << (currentThread->getName()) << std::endl;
        (void) interrupt->SetLevel(old);
        return;
    }//end of if not owner
    if(!lockWaitQueue.empty()){         //check if there is a thread waiting on the lock
        owner = lockWaitQueue.front();  //get next thread in the wait queue
        lockWaitQueue.erase(lockWaitQueue.begin());           //removing thread from wait queue
        scheduler->ReadyToRun(owner);   //setting the next thread to be the owner
    }//end of if
    else{
        BUSY = false;                   //no threads in the wait queue so set BUSY to false (lock is available)
        owner = NULL;
    }
    (void) interrupt->SetLevel(old);     //retore interrupts
    return;
}

Condition::Condition(char* debugName) { 
    waitingLock = NULL;
    name = debugName;
}
Condition::~Condition() { 
    delete waitingLock;
    for(unsigned int i=0; i<cvWaitQueue.size(); i++){
        //delete[] cvWaitQueue;
    }
}
void Condition::Wait(Lock* conditionLock) { 
    //ASSERT(FALSE); 
    IntStatus old = interrupt->SetLevel(IntOff);    //first set interrupts off
    if(conditionLock == NULL){
        std::cout << " >> Error!  Recieved a NULL lock in Condition::Wait(...)" << std::endl;
        (void) interrupt->SetLevel(old);
        return;
    }//end of if null lock
    if(waitingLock == NULL){
        waitingLock = conditionLock;
    }
    if(waitingLock != conditionLock){
        std::cout << " >> Error!  Lock does not match current waiting Lock in Condition::Wait(...)" << std::endl;
        (void) interrupt->SetLevel(old);
        return;
    }//end of if
    cvWaitQueue.push_back(currentThread);
    conditionLock->Release();
    currentThread->Sleep();
    conditionLock->Acquire();
    (void) interrupt->SetLevel(old);
    return;
}
void Condition::Signal(Lock* conditionLock) { 
    IntStatus old = interrupt->SetLevel(IntOff);    //first set interrupts off
    if(waitingLock == NULL){        
        (void) interrupt->SetLevel(old);
        return;
    }
    if(waitingLock != conditionLock){
         std::cout << " >> Error!  Lock does not match current waiting Lock in Condition::Signal(...)" << std::endl;
        (void) interrupt->SetLevel(old);
        return;
    }
    Thread *t = cvWaitQueue.front();
    cvWaitQueue.erase(cvWaitQueue.begin());
    scheduler->ReadyToRun(t);
    if(cvWaitQueue.empty()){
        waitingLock = NULL;
    }
    (void) interrupt->SetLevel(old);
    return;
}
void Condition::Broadcast(Lock* conditionLock) { 
    IntStatus old = interrupt->SetLevel(IntOff);    //first set interrupts off
     if(conditionLock == NULL){
        (void) interrupt->SetLevel(old);
        return;
     }
     if(waitingLock != conditionLock){
         std::cout << " >> Error!  Lock does not match current waiting Lock in Condition::Broadcast(...)" << std::endl;
        (void) interrupt->SetLevel(old);
        return;
    }
    (void) interrupt->SetLevel(old);
    while(!cvWaitQueue.empty()){
        Signal(conditionLock);
    }
}
