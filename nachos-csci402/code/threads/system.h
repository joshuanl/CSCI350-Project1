// system.h 
//	All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SYSTEM_H
#define SYSTEM_H

#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"
#include <map>

// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization,
						// called before anything else
extern void Cleanup();				// Cleanup, called when
						// Nachos is done.

extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock

#define NUM_LOCKS 50
#define NUM_CONDITIONS 50

class AddrSpace;
class Condition;

//Create Kernel Lock Class
struct KernelLock {
	Lock lock;
	AddrSpace* address;
	uint32_t toDelete; //0 = no, 1 = yes
	uint32_t threads; // Represents number of threads
	char* name;
};

//Create Kernel Condition Class
struct KernelCondition {
	Condition* condition;
	AddrSpace* address;
	uint32_t toDelete; //0=no, 1 = yes
	uint32_t threads; //Represents number of threads
	char* name;
};

extern std::map<AddrSpace*, uint32_t> threadTable;
extern Lock tableLock;
extern KernelLock lockTable[NUM_LOCKS];
extern Lock ctableLock;
extern KernelCondition* cTable[NUM_CONDITIONS];



#ifdef USER_PROGRAM
#include "machine.h"
extern Machine* machine;	// user program memory and registers
#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB 
#include "filesys.h"
extern FileSystem  *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk   *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice* postOffice;
#endif

#endif // SYSTEM_H
