// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.


/*
	Need to do:

		- need to add run() for clerks 
		- create interactions between client - application clerk first 
	
	How to share/communicate data between customer/clerk? 
	 - customers should be created with index as a member variable // need to add
	 - create a global vector of the class 	//need to add
	 	- when customer acquires lock from clerk, customer passes clerk their index	with a function
	 		- store index in a vector // should be same size as lineCount
		

*/





#include "copyright.h"
#include "system.h"
#include "synch.h"
#include <cmath>
#include <time.h>
#include <map>
#include <iostream>
#include <queue>

//PROTOTYPES
class Client;
class ApplicationClerk;
class PictureClerk;
class PassPortClerk;
class Cashier;	 		

struct ApplicationMonitor;
struct PictureMonitor;
struct PassportMonitor;

// GLOBAL VARIABLES FOR PROBLEM 2
int ssnCount = 0;
const int clientStartMoney[4] = {100, 500, 1100, 1600};
ApplicationMonitor* AMonitor;
PictureMonitor* PMonitor;
PassportMonitor* PPMonitor;


std::vector<ApplicationClerk *> aClerks;
std::vector<PictureClerk *> pClerks;
std::vector<PassportClerk *> ppClerks;
std::vector<Cashier *> cClerks;
std::vector<Client *> customers; //DO NOT POP CUSTOMERS FROM THIS VECTOR. 
//OTHERWISE WE WILL HAVE TO REINDEX THE CUSTOMERS AND THAT IS A BIG PAIN 

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------



void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ThreadTest
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest()
{
    DEBUG('t', "Entering SimpleTest");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);
}



// #include "copyright.h"
// #include "system.h"
#ifdef CHANGED
#include "synch.h"
#endif

#ifdef CHANGED
// --------------------------------------------------
// Test Suite
// --------------------------------------------------


// --------------------------------------------------
// Test 1 - see TestSuite() for details
// --------------------------------------------------
Semaphore t1_s1("t1_s1",0);       // To make sure t1_t1 acquires the
                                  // lock before t1_t2
Semaphore t1_s2("t1_s2",0);       // To make sure t1_t2 Is waiting on the 
                                  // lock before t1_t3 releases it
Semaphore t1_s3("t1_s3",0);       // To make sure t1_t1 does not release the
                                  // lock before t1_t3 tries to acquire it
Semaphore t1_done("t1_done",0);   // So that TestSuite knows when Test 1 is
                                  // done
Lock t1_l1("t1_l1");		  // the lock tested in Test 1

// --------------------------------------------------
// t1_t1() -- test1 thread 1
//     This is the rightful lock owner
// --------------------------------------------------
void t1_t1() {
    t1_l1.Acquire();
    t1_s1.V();  // Allow t1_t2 to try to Acquire Lock
 
    printf ("%s: Acquired Lock %s, waiting for t3\n",currentThread->getName(),
	    t1_l1.getName());
    t1_s3.P();
    printf ("%s: working in CS\n",currentThread->getName());
    for (int i = 0; i < 1000000; i++) ;
    printf ("%s: Releasing Lock %s\n",currentThread->getName(),
	    t1_l1.getName());
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t2() -- test1 thread 2
//     This thread will wait on the held lock.
// --------------------------------------------------
void t1_t2() {

    t1_s1.P();	// Wait until t1 has the lock
    t1_s2.V();  // Let t3 try to acquire the lock

    printf("%s: trying to acquire lock %s\n",currentThread->getName(),
	    t1_l1.getName());
    t1_l1.Acquire();

    printf ("%s: Acquired Lock %s, working in CS\n",currentThread->getName(),
	    t1_l1.getName());
    for (int i = 0; i < 10; i++)
	;
    printf ("%s: Releasing Lock %s\n",currentThread->getName(),
	    t1_l1.getName());
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t3() -- test1 thread 3
//     This thread will try to release the lock illegally
// --------------------------------------------------
void t1_t3() {

    t1_s2.P();	// Wait until t2 is ready to try to acquire the lock

    t1_s3.V();	// Let t1 do it's stuff
    for ( int i = 0; i < 3; i++ ) {
	printf("%s: Trying to release Lock %s\n",currentThread->getName(),
	       t1_l1.getName());
	t1_l1.Release();
    }
}

// --------------------------------------------------
// Test 2 - see TestSuite() for details
// --------------------------------------------------
Lock t2_l1("t2_l1");		// For mutual exclusion
Condition t2_c1("t2_c1");	// The condition variable to test
Semaphore t2_s1("t2_s1",0);	// To ensure the Signal comes before the wait
Semaphore t2_done("t2_done",0);     // So that TestSuite knows when Test 2 is
                                  // done

// --------------------------------------------------
// t2_t1() -- test 2 thread 1
//     This thread will signal a variable with nothing waiting
// --------------------------------------------------
void t2_t1() {
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
	   t2_l1.getName(), t2_c1.getName());
    t2_c1.Signal(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t2_l1.getName());
    t2_l1.Release();
    t2_s1.V();	// release t2_t2
    t2_done.V();
}

// --------------------------------------------------
// t2_t2() -- test 2 thread 2
//     This thread will wait on a pre-signalled variable
// --------------------------------------------------
void t2_t2() {
    t2_s1.P();	// Wait for t2_t1 to be done with the lock
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t2_l1.getName(), t2_c1.getName());
    t2_c1.Wait(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t2_l1.getName());
    t2_l1.Release();
}
// --------------------------------------------------
// Test 3 - see TestSuite() for details
// --------------------------------------------------
Lock t3_l1("t3_l1");		// For mutual exclusion
Condition t3_c1("t3_c1");	// The condition variable to test
Semaphore t3_s1("t3_s1",0);	// To ensure the Signal comes before the wait
Semaphore t3_done("t3_done",0); // So that TestSuite knows when Test 3 is
                                // done

// --------------------------------------------------
// t3_waiter()
//     These threads will wait on the t3_c1 condition variable.  Only
//     one t3_waiter will be released
// --------------------------------------------------
void t3_waiter() {
    t3_l1.Acquire();
    t3_s1.V();		// Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t3_l1.getName(), t3_c1.getName());
    t3_c1.Wait(&t3_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t3_c1.getName());
    t3_l1.Release();
    t3_done.V();
}


// --------------------------------------------------
// t3_signaller()
//     This threads will signal the t3_c1 condition variable.  Only
//     one t3_signaller will be released
// --------------------------------------------------
void t3_signaller() {

    // Don't signal until someone's waiting
    
    for ( int i = 0; i < 5 ; i++ ) 
	t3_s1.P();
    t3_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
	   t3_l1.getName(), t3_c1.getName());
    t3_c1.Signal(&t3_l1);
    printf("%s: Releasing %s\n",currentThread->getName(), t3_l1.getName());
    t3_l1.Release();
    t3_done.V();
}
 
// --------------------------------------------------
// Test 4 - see TestSuite() for details
// --------------------------------------------------
Lock t4_l1("t4_l1");		// For mutual exclusion
Condition t4_c1("t4_c1");	// The condition variable to test
Semaphore t4_s1("t4_s1",0);	// To ensure the Signal comes before the wait
Semaphore t4_done("t4_done",0); // So that TestSuite knows when Test 4 is
                                // done

// --------------------------------------------------
// t4_waiter()
//     These threads will wait on the t4_c1 condition variable.  All
//     t4_waiters will be released
// --------------------------------------------------
void t4_waiter() {
    t4_l1.Acquire();
    t4_s1.V();		// Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t4_l1.getName(), t4_c1.getName());
    t4_c1.Wait(&t4_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t4_c1.getName());
    t4_l1.Release();
    t4_done.V();
}


// --------------------------------------------------
// t2_signaller()
//     This thread will broadcast to the t4_c1 condition variable.
//     All t4_waiters will be released
// --------------------------------------------------
void t4_signaller() {

    // Don't broadcast until someone's waiting
    
    for ( int i = 0; i < 5 ; i++ ) 
	t4_s1.P();
    t4_l1.Acquire();
    printf("%s: Lock %s acquired, broadcasting %s\n",currentThread->getName(),
	   t4_l1.getName(), t4_c1.getName());
    t4_c1.Broadcast(&t4_l1);
    printf("%s: Releasing %s\n",currentThread->getName(), t4_l1.getName());
    t4_l1.Release();
    t4_done.V();
}
// --------------------------------------------------
// Test 5 - see TestSuite() for details
// --------------------------------------------------
Lock t5_l1("t5_l1");		// For mutual exclusion
Lock t5_l2("t5_l2");		// Second lock for the bad behavior
Condition t5_c1("t5_c1");	// The condition variable to test
Semaphore t5_s1("t5_s1",0);	// To make sure t5_t2 acquires the lock after
                                // t5_t1

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a condition under t5_l1
// --------------------------------------------------
void t5_t1() {
    t5_l1.Acquire();
    t5_s1.V();	// release t5_t2
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t5_l1.getName(), t5_c1.getName());
    t5_c1.Wait(&t5_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t5_l1.getName());
    t5_l1.Release();
}

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a t5_c1 condition under t5_l2, which is
//     a Fatal error
// --------------------------------------------------
void t5_t2() {
    t5_s1.P();	// Wait for t5_t1 to get into the monitor
    t5_l1.Acquire();
    t5_l2.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
	   t5_l2.getName(), t5_c1.getName());
    t5_c1.Signal(&t5_l2);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t5_l2.getName());
    t5_l2.Release();
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t5_l1.getName());
    t5_l1.Release();
}

// --------------------------------------------------
// TestSuite()
//     This is the main thread of the test suite.  It runs the
//     following tests:
//
//       1.  Show that a thread trying to release a lock it does not
//       hold does not work
//
//       2.  Show that Signals are not stored -- a Signal with no
//       thread waiting is ignored
//
//       3.  Show that Signal only wakes 1 thread
//
//	 4.  Show that Broadcast wakes all waiting threads
//
//       5.  Show that Signalling a thread waiting under one lock
//       while holding another is a Fatal error
//
//     Fatal errors terminate the thread in question.
// --------------------------------------------------
void TestSuite() {
    Thread *t;
    char *name;
    int i;
    
    // Test 1

    printf("Starting Test 1\n");

    t = new Thread("t1_t1");
    t->Fork((VoidFunctionPtr)t1_t1,0);

    t = new Thread("t1_t2");
    t->Fork((VoidFunctionPtr)t1_t2,0);

    t = new Thread("t1_t3");
    t->Fork((VoidFunctionPtr)t1_t3,0);

    // Wait for Test 1 to complete
    for (  i = 0; i < 2; i++ )
	t1_done.P();

    // Test 2

    printf("Starting Test 2.  Note that it is an error if thread t2_t2\n");
    printf("completes\n");

    t = new Thread("t2_t1");	
    t->Fork((VoidFunctionPtr)t2_t1,0);

    t = new Thread("t2_t2");
    t->Fork((VoidFunctionPtr)t2_t2,0);

    // Wait for Test 2 to complete
    t2_done.P();

    // Test 3

    printf("Starting Test 3\n");

    for (  i = 0 ; i < 5 ; i++ ) {
	name = new char [20];
	sprintf(name,"t3_waiter%d",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)t3_waiter,0);
    }
    t = new Thread("t3_signaller");
    t->Fork((VoidFunctionPtr)t3_signaller,0);

    // Wait for Test 3 to complete
    for (  i = 0; i < 2; i++ )
	t3_done.P();

    // Test 4

    printf("Starting Test 4\n");

    for (  i = 0 ; i < 5 ; i++ ) {
	name = new char [20];
	sprintf(name,"t4_waiter%d",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)t4_waiter,0);
    }
    t = new Thread("t4_signaller");
    t->Fork((VoidFunctionPtr)t4_signaller,0);

    // Wait for Test 4 to complete
    for (  i = 0; i < 6; i++ )
	t4_done.P();

    // Test 5

    printf("Starting Test 5.  Note that it is an error if thread t5_t1\n");
    printf("completes\n");

    t = new Thread("t5_t1");
    t->Fork((VoidFunctionPtr)t5_t1,0);

    t = new Thread("t5_t2");
    t->Fork((VoidFunctionPtr)t5_t2,0);

}
#endif


struct ApplicationMonitor {

	Lock* AMonitorLock;

	int numAClerks;
	Lock** clerkLineLocks;					// move to be global variable
	Condition** clerkLineCV;
	Condition** clerkBribeLineCV;

	int* clerkLineCount;
	int* clerkBribeLineCount;
	int* clerkState;	//0: available     1: busy    2: on break

	std::queue<int>* clientSSNs;

	ApplicationMonitor(int numApplicationClerks, int numCustomers)
	{
		AMonitorLock = new Lock("Monitor Lock");

		numAClerks = numApplicationClerks;
		clerkLineLocks = new Lock*[numAClerks];
		clerkLineCV = new Condition*[numAClerks];
		clerkBribeLineCV = new Condition*[numAClerks];
		
		clerkLineCount = new int[numAClerks];
		clerkBribeLineCount = new int[numAClerks];
		clerkState = new int[numAClerks];

		clientSSNs = new std::queue<int>[numCustomers];
		

		for(int i = 0; i < numAClerks; i++)
		{			
			clerkLineCV[i] = new Condition("");
			clerkBribeLineCV[i] = new Condition("");
			clerkLineLocks[i] = new Lock("ClerkLineLock");
			clerkLineCount[i] = 0;
			clerkBribeLineCount[i] = 0;
			clerkState[i] = 0;
		}
	}//end of constructor

	~ApplicationMonitor(){

	}//end of destructor	

	int getSmallestLine()
	{
		int smallest = 50;
		int smallestIndex = 0;
		//std::cout << "num clerks: " << numAClerks << std::endl;
		for(int i = 0; i < numAClerks; i++)
		{
			std::cout << clerkLineCount[i] << std::endl;
			if(clerkLineCount[i] < smallest)
			{
				smallest = clerkLineCount[i];
				smallestIndex = i;
			}
		}
		return smallestIndex;
	}

	void giveSSN(int line, int ssn)
	{
		clientSSNs[line].push(ssn);
	}
};

struct PictureMonitor {

	Lock* PMonitorLock;

	int numPClerks;
	Lock** clerkLineLocks;					// move to be global variable
	Condition** clerkLineCV;
	Condition** clerkBribeLineCV;

	int* clerkLineCount;
	int* clerkBribeLineCount;
	int* clerkState;	//0: available     1: busy    2: on break
	std::queue<int>* clientSSNs;

	PictureMonitor(int numPictureClerks, int numCustomers)
	{
		PMonitorLock = new Lock("Monitor Lock");

		numPClerks = numPictureClerks;
		clerkLineLocks = new Lock*[numPClerks];
		clerkLineCV = new Condition*[numPClerks];
		clerkBribeLineCV = new Condition*[numPClerks];
		
		clerkLineCount = new int[numPClerks];
		clerkBribeLineCount = new int[numPClerks];
		clerkState = new int[numPClerks];

		clientSSNs = new std::queue<int>[numCustomers];

		for(int i = 0; i < numPClerks; i++)
		{			
			clerkLineCV[i] = new Condition("");
			clerkBribeLineCV[i] = new Condition("");
			clerkLineLocks[i] = new Lock("ClerkLineLock");
			clerkLineCount[i] = 0;
			clerkBribeLineCount[i] = 0;
			clerkState[i] = 0;
		}
	}//end of constructor

	~PictureMonitor(){

	}//end of destructor	

	int getSmallestLine()
	{
		int smallest = 50;
		int smallestIndex = 0;
		for(int i = 0; i < numPClerks; i++)
		{
			//std::cout << clerkLineCount[i] << std::endl;
			if(clerkLineCount[i] < smallest)
			{
				smallest = clerkLineCount[i];
				smallestIndex = i;
			}
		}
		return smallestIndex;
	}

	void giveSSN(int line, int ssn)
	{
		clientSSNs[line].push(ssn);
	}

};

class Client {

private:
	int money;
	int id;
	int ssn;
    int selfIndex;
	bool applicationAccepted;
	bool pictureTaken;
	bool bribed; 	//reset after each line
public:
	Client(int num, int startMoney){

		id = num;
		ssn = id;
		money = startMoney;
        selfIndex = 0; //defines position in the customer vector
		std::cout << "ssn: " << ssn << "  startMoney: " << startMoney << std::endl;

		applicationAccepted = false;
		pictureTaken = false;
		bribed = false;

		//need to randomize
		joinApplicationLine();

		joinPictureLine();

		joinPassportLine();
	}//end of client constructor

	~Client(){
        //Adding code to reindex customers vector after deleting a client

	}//end of client deconstructor

	void joinApplicationLine()
	{
		AMonitor->AMonitorLock->Acquire();
		int whichLine = AMonitor->getSmallestLine();

		
		std::cout << whichLine << std::endl;
		AMonitor->clerkLineCount[whichLine] += 1;	
		AMonitor->giveSSN(whichLine, ssn);	

		std::cout << "Customer " << id << " has gotten in regular line for ApplicationClerk " << whichLine << "." << std::endl;
		
		//AMonitor->clerkLineLocks[whichLine]->Release();

		AMonitor->clerkLineLocks[whichLine]->Acquire();

		AMonitor->AMonitorLock->Release();

		AMonitor->clerkLineCV[whichLine]->Wait(AMonitor->clerkLineLocks[whichLine]);
		
		std::cout << "Customer " << id << " has given SSN " << ssn << " to Application Clerk " << whichLine << std::endl;

		AMonitor->clerkLineCV[whichLine]->Signal(AMonitor->clerkLineLocks[whichLine]);

		AMonitor->clerkLineCV[whichLine]->Wait(AMonitor->clerkLineLocks[whichLine]);

		applicationAccepted = true;
		
		AMonitor->clerkLineLocks[whichLine]->Release();	
	}

	void joinPictureLine()
	{
		PMonitor->PMonitorLock->Acquire();
        int whichLine = PMonitor->getSmallestLine();
		std::cout << whichLine << std::endl;
		PMonitor->clerkLineCount[whichLine] += 1;
		PMonitor->giveSSN(whichLine, ssn);	
		std::cout << "Customer " << id << " has gotten in regular line for PictureClerk " << whichLine << "." << std::endl;
		PMonitor->clerkLineLocks[whichLine]->Acquire();
        PMonitor->PMonitorLock->Release();
        PMonitor->clerkLineCV[whichLine]->Wait(PMonitor->clerkLineLocks[whichLine]);
        std::cout << "Customer " << id << " has given SSN " << ssn << " to Picture Clerk " << whichLine << std::endl;
        PMonitor->clerkLineCV[whichLine]->Signal(PMonitor->clerkLineLocks[whichLine]);
        PMonitor->clerkLineCV[whichLine]->Wait(PMonitor->clerkLineLocks[whichLine]);
		pictureTaken = true;
        PMonitor->clerkLineLocks[whichLine]->Release();
	}


	/*void joinPassportLine()
	{
		PPMonitor->MonitorLock->Acquire();
		
		int whichLine = PPMonitor->getSmallestLine();
		
		PPMonitor->clerkLineLocks[whichLine]->Acquire();
		std::cout << whichLine << std::endl;
		PPMonitor->clerkLineCount[whichLine] += 1;
		std::cout << PPMonitor->clerkLineCount[whichLine] << " in line" << std::endl;
		PPMonitor->giveSSN(whichLine, ssn);	
		PPMonitor->giveReqs(whichLine, ssn);
		int lineSpot = PPMonitor->clerkLineCount[whichLine];

		std::cout << "Customer " << id << " has gotten in regular line for Passport Clerk " << whichLine << "." << std::endl;
		
		PPMonitor->MonitorLock->Release();
		//std::cout << "Released picture monitor lock" << std::endl;

		while(lineSpot > 0)
		{
			//PPMonitor->clerkLineLocks[whichLine]->Acquire();
			PPMonitor->clerkLineCV[whichLine]->Wait(PPMonitor->clerkLineLocks[whichLine]);			
			lineSpot--;
			
		}
		std::cout << "Customer " << id << " has given SSN " << ssn << " to Passport Clerk " << whichLine << std::endl;
		PPMonitor->clerkLineLocks[whichLine]->Release();
	}*/

    void joinPassportLine() {
        PPMonitor->PPMonitorLock->Acquire();
        int whichLine = PPMonitor->getSmallestLine();
        std::cout << whichLine << std::endl;
        PPMonitor->clerkLineCount[whichLine] += 1;
        PPMonitor->giveSSN(whichLine, ssn);
        int completed = 0;
        if(applicationAccepted)
        {
            completed++;
        }
        if(pictureTaken)
        {
            completed++;
        }
        PPMonitor->giveReqs(whichLine, completed);
        std::cout << "Customer " << id << " has gotten in regular line for Passport Clerk " << whichLine << "." << std::endl;
        PPMonitor->clerkLineLocks[whichLine]->Acquire();
        PPMonitor->PPMonitorLock->Release();
        PPMonitor->clerkLineCV[whichLine]->Wait(PPMonitor->clerkLineLocks[whichLine]);
        std::cout << "Customer " << id << " has given SSN " << ssn << " to Passport Clerk " << whichLine << std::endl;
        PPMonitor->clerkLineCV[whichLine]->Signal(PPMonitor->clerkLineLocks[whichLine]);
        PPMonitor->clerkLineCV[whichLine]->Wait(PPMonitor->clerkLineLocks[whichLine]);
        //Something should be declared true here
        PPMonitor->clerkLineLocks[whichLine]->Release();
        
    }

    void joinCashierLine() {
    CashierMonitor->CashierMonitorLock->Acquire();
        int whichLine = CashierMonitor->getSmallestLine();
        std::cout << whichLine << std::endl;
        CashierMonitor->clerkLineCount[whichLine] += 1;
        CashierMonitor->giveSSN(whichLine, ssn);
        std::cout << "Customer " << id << " has gotten in regular line for Cashier " << whichLine << "." << std::endl;
        CashierMonitor->clerkLineLocks[whichLine]->Acquire();
        CashierMonitor->CashierMonitorLock->Release();
        CashierMonitor->clerkLineCV[whichLine]->Wait(CashierMonitor->clerkLineLocks[whichLine]);
        std::cout << "Customer " << id << " has given SSN " << ssn << " to Cashier " << whichLine << std::endl;
        CashierMonitor->clerkLineCV[whichLine]->Signal(CashierMonitor->clerkLineLocks[whichLine]);
        CashierMonitor->clerkLineCV[whichLine]->Wait(CashierMonitor->clerkLineLocks[whichLine]);
        //Something should be declared true here
        CashierMonitor->clerkLineLocks[whichLine]->Release();
        
    }
>>>>>>> 6cf6ebd943932e264071d895ba47cdae574d075b

>>>>>>> 8a1304794a473b00b7e5db3a065d174affa8d00f

	void moveUpInLine(){
		if(money >= 600){
			money -= 500;
			bribed = true;
		}
	}//end of move up in line

	void setAppAccepted(bool b){
		applicationAccepted = b;
	}

	void setPictureTaken(bool b){
		pictureTaken = b;
	}

    void setselfIndex (int i) {
        selfIndex = i;
    } //Setter for self-index

    int getselfIndex () {
        return selfIndex;
    }



	bool isAppAccepted(){
		return applicationAccepted;
	}//end of isappaccepted

	bool isPictureTaken(){
		return pictureTaken;
	}//end of of is picture taken

	bool alreadyBribed(){
		return bribed;
	}//end of br

};  //end of client class


class ApplicationClerk {
private:
	int clerkState; // 0: available     1: busy       2: on break
	int lineCount;   
	int bribeLineCount;
	int clerkMoney; //How much money the clerk has
	int myLine;
	//std::vector<Client*> myLine;

public:
	ApplicationClerk(){
		clerkState = 0;
		lineCount = 0;
		bribeLineCount = 0;
		clerkMoney = 0;
		myLine = -1;

	}//end of constructor

	~ApplicationClerk(){

	}//endo of deconstructor

	void run(){		
		while(true)
		{
			
			AMonitor->clerkLineLocks[myLine]->Acquire();

			if(AMonitor->clerkBribeLineCount[myLine] > 0)
			{
				//AMonitor->clerkBribeLineCV[myLine]->Signal(AMonitor->clerkLineLocks[myLine]);
				//AMonitor->clerkState[myLine] = 1;
			}
			else if(AMonitor->clerkLineCount[myLine] > 0)
			{       //if bribe line is empty
				
				AMonitor->clerkLineCV[myLine]->Signal(AMonitor->clerkLineLocks[myLine]); 
				std::cout << "Application Clerk " << myLine << " has signalled a Customer to come to their counter." << std::endl;
				AMonitor->clerkState[myLine] = 1;

				AMonitor->clerkLineCV[myLine]->Wait(AMonitor->clerkLineLocks[myLine]); 

				//wait
				std::cout << "Application Clerk " << myLine << " has received SSN " << AMonitor->clientSSNs[myLine].front() <<
					" from Customer " << AMonitor->clientSSNs[myLine].front() << "." << std::endl;
				AMonitor->clientSSNs[myLine].pop();				
				AMonitor->clerkLineCount[myLine]--;
				std::cout << "" << AMonitor->clerkLineCount[myLine] << " customers left in line " << myLine << std::endl;

				AMonitor->clerkLineCV[myLine]->Signal(AMonitor->clerkLineLocks[myLine]); 
			}
			else
			{
				//std::cout << "ApplicationClerk " << myLine << "is going on break." << std::endl;
				//AMonitor->clerkState[myLine] = 0;
			}

			AMonitor->clerkLineLocks[myLine]->Release();
		
		}
		
	}


	int getclerkState(){
		return clerkState;
	}//end of getclerkState

	void setclerkState(int n){
		clerkState = n;
	}//end of setting clerkState

	void setselfIndex (int i) {
        myLine = i;
    } //Setter for self-index

	int getLineCount()
	{
		return lineCount;
	}

	void addToLine()
	{
		//myLine.push_back(client);
		lineCount++;
	}//end of adding to line

	void addToBribeLine(){
		bribeLineCount++;
	}//end of adding to bribe line

	void addClerkMoney(int n){
		clerkMoney = clerkMoney + n;
	}//Adding money to clerk money variab;e

	int getclerkMoney(){
		return clerkMoney;
	}//Get clerk money

	void goOnBreak(){
		clerkState = 2;
		//send to sleep		
		currentThread->Sleep();	
	}//end of sending clerk to break;

	void goBackToWork(){
		clerkState = 1;
		//wake up from sleep

	}//end of going back to work

	void makeAvailable(){
		clerkState = 0;
	} //set clerk state to available
}; //end of class

class PictureClerk {
private:
	int clerkState; // 0: available     1: busy       2: on break
	int lineCount;   
	int bribeLineCount;
	int clerkMoney; //How much money the clerk has
	int myLine;
	//std::vector<Client*> myLine;

public:
	PictureClerk()
	{
		clerkState = 0;
		lineCount = 0;
		bribeLineCount = 0;
		clerkMoney = 0;
		myLine = -1;

	}//end of constructor

	~PictureClerk(){

	}//endo of deconstructor

	void run(){	
		while(true)
		{
			
			if(PMonitor->clerkBribeLineCount[myLine] > 0)
			{
				PMonitor->clerkBribeLineCV[myLine]->Signal(PMonitor->clerkLineLocks[myLine]);
				PMonitor->clerkState[myLine] = 1;
			}
			else if(PMonitor->clerkLineCount[myLine] > 0)
			{       //if bribe line is empty
				PMonitor->clerkState[myLine] = 1;
				std::cout << "PictureClerk " << myLine << " has received SSN " << PMonitor->clientSSNs[myLine].front() <<
					" from Customer " << PMonitor->clientSSNs[myLine].front() << "." << std::endl;	
				PMonitor->clientSSNs[myLine].pop();	
				PMonitor->clerkLineCount[myLine]--;
				std::cout << "" << PMonitor->clerkLineCount[myLine] << " customers left in line " << myLine << std::endl;
				
				PMonitor->clerkLineCV[myLine]->Signal(PMonitor->clerkLineLocks[myLine]); 
				std::cout << "PictureClerk " << myLine << " has signalled a Customer to come to their counter." << std::endl;
				
			}
			else
			{
				//std::cout << "PictureClerk " << myLine << " is going on break." << std::endl;
				//PMonitor->clerkState[myLine] = 0;
			}
		}//end of while
		
	}


	int getclerkState(){
		return clerkState;
	}//end of getclerkState

	void setclerkState(int n){
		clerkState = n;
	}//end of setting clerkState

	void setselfIndex (int i) {
        myLine = i;
    } //Setter for self-index

	int getLineCount()
	{
		return lineCount;
	}

	void addToLine()
	{
		//myLine.push_back(client);
		lineCount++;
	}//end of adding to line

	void addToBribeLine(){
		bribeLineCount++;
	}//end of adding to bribe line

	void addClerkMoney(int n){
		clerkMoney = clerkMoney + n;
	}//Adding money to clerk money variab;e

	int getclerkMoney(){
		return clerkMoney;
	}//Get clerk money

	void goOnBreak(){
		clerkState = 2;
		//send to sleep		
		currentThread->Sleep();	
	}//end of sending clerk to break;

	void goBackToWork(){
		clerkState = 1;
		//wake up from sleep

	}//end of going back to work

	void makeAvailable(){
		clerkState = 0;
	} //set clerk state to available
}; //end of class

class PassportClerk {
private:
	int clerkState; // 0: available     1: busy       2: on break
	int lineCount;   
	int bribeLineCount;
	int clerkMoney; //How much money the clerk has
	int myLine;

public:

	PassportClerk(int line){
		clerkState = 0;
		lineCount = 0;
		bribeLineCount = 0;
		clerkMoney = 0;
		myLine = line;
	}//end of constructor

	~PassportClerk(){

	}//end of deconstructor

	void run()
	{	
		while(true)
		{
			//std::cout << "should be here" << std::endl;
			//PMonitor->PMonitorLock->Acquire();
			//std::cout << "should not be here" << std::endl;

			PPMonitor->clerkLineLocks[myLine]->Acquire();
			if(PPMonitor->clerkBribeLineCount[myLine] > 0)
			{
				//PMonitor->clerkBribeLineCV[myLine]->Signal(PMonitor->clerkLineLocks[myLine]);
				//PMonitor->clerkState[myLine] = 1;
			}
			else if(PPMonitor->clerkLineCount[myLine] > 0)
			{       //if bribe line is empty

				PPMonitor->clerkLineCV[myLine]->Signal(PPMonitor->clerkLineLocks[myLine]); 
				std::cout << "Passport Clerk " << myLine << " has signalled a Customer to come to their counter." << std::endl;

				PPMonitor->clerkState[myLine] = 1;

				std::cout << "Passport Clerk " << myLine << " has received SSN " << PPMonitor->clientSSNs[myLine].front() <<
					" from Customer " << PPMonitor->clientSSNs[myLine].front() << "." << std::endl;	

				if(PPMonitor->clientReqs[myLine].front() != 2)
				{
					std::cout << "PassportClerk " << myLine << " has determined that Customer " << PPMonitor->clientSSNs[myLine].front() << " does not have both their application and picture completed." << std::endl;
				}
				else
				{
					std::cout << "PassportClerk " << myLine << " has determined that Customer " << PPMonitor->clientSSNs[myLine].front() << " has both their application and picture completed." << std::endl;
					std::cout << "PassportClerk " << myLine << " has recorded Customer " << PPMonitor->clientSSNs[myLine].front() << " passport information." << std::endl;
				}
				
				PPMonitor->clientSSNs[myLine].pop();	
				PPMonitor->clientReqs[myLine].pop();
				PPMonitor->clerkLineCount[myLine]--;
				std::cout << "" << PPMonitor->clerkLineCount[myLine] << " customers left in line " << myLine << std::endl;
				
				
				
			}
			else
			{
				//std::cout << "PictureClerk " << myLine << " is going on break." << std::endl;
				//PMonitor->clerkState[myLine] = 0;
			}
			//PMonitor->PMonitorLock->Release();
			//PMonitor->clerkLineLocks[myLine]->Acquire();
			PPMonitor->clerkLineLocks[myLine]->Release();
		}//end of while
	}


	int getclerkState(){
		return clerkState;
	}//end of getclerkState

	void setclerkState(int n){
		clerkState = n;
	}//end of setting clerkState

	void addToLine(){
		lineCount++;
	}//end of adding to line

	void addToBribeLine(){
		bribeLineCount++;
	}//end of adding to bribe line

	void addclerkMoney(int n){
		clerkMoney = clerkMoney + n;
	}//Adding money to clerk money variab;e

	int getclerkMoney(){
		return clerkMoney;
	}//Get clerk money

	void goOnBreak(){
		clerkState = 2;
		//send to sleep

	}//end of sending clerk to break;

	void goBackToWork(){
		clerkState = 1;
		//wake up from sleep

	}//end of going back to work

	void makeAvailable(){
		clerkState = 0;
	} //set clerk state to available

}; // end of passport clerk	


class Manager {
private:
	
	int pClerkMoney;
	int aClerkMoney;
	int ppClerkMoney;
	int cClerkMoney;
	int totalMoney;
public:
	Manager() {
		pClerkMoney = 0;
		aClerkMoney = 0;
		ppClerkMoney = 0;
		cClerkMoney = 0;
		totalMoney = 0;

	}//end of constructor

	~Manager(){

	}//end of deconstructor

	void wakeupClerks(){

	}//end of waking up clerks
    
	void updateTotalMoney(){
		pClerkMoney = 0;
		aClerkMoney = 0;
		ppClerkMoney = 0;
		cClerkMoney = 0;
		for(unsigned int i=0; i < aClerks.size(); i++){		
			aClerkMoney += aClerks[i]->getclerkMoney();
		}//end of for
		for(unsigned int i=0; i < pClerks.size(); i++){
			pClerkMoney += pClerks[i]->getclerkMoney();
		}//end of for
		for(unsigned int i=0; i < ppClerks.size(); i++){
			ppClerkMoney += ppClerks[i]->getclerkMoney();
		}//end of for
		for(unsigned int i=0; i < cClerks.size(); i++){
			cClerkMoney += cClerks[i]->getclerkMoney();
		}//end of for
		totalMoney = pClerkMoney + aClerkMoney + ppClerkMoney + cClerkMoney;
	}//end of updating total money 

    int getaClerkMoney() {
        updateTotalMoney();
        std::cout << "Manager has counted a total of $" << aClerkMoney << " for ApplicationClerks" << std::endl;
        return aClerkMoney;
    }
    
    int getpClerkMoney() {
        updateTotalMoney();
        std::cout << "Manager has counted a total of $" << pClerkMoney << " for PictureClerks" << std::endl;
        return pClerkMoney;
    }
    
    int getppClerkMoney() {
        updateTotalMoney();
        std::cout << "Manager has counted a total of $" << ppClerkMoney << " for PassportClerks" << std::endl;
        return ppClerkMoney;
    }
    
    int getcClerkMoney() {
        updateTotalMoney();
        std::cout << "Manager has counted a total of $" << cClerkMoney << " for Cashiers" << std::endl;
        return cClerkMoney;
    }
    
    int gettotalMoney() {
        updateTotalMoney();
        std::cout << "Manager has counted a total of $" << totalMoney << " for the passport office" << std::endl;
        return totalMoney;
    } //End of getters for different clerk money

}; //end of manager class


class Senator {
private:
	int money;
	int ssn;
	bool applicationAccepted;
	bool pictureTaken;
	bool bribed;
public:

	Senator(int sn, int startMoney){
		ssn = sn;
		money = startMoney;
	}//end of constructor

	~Senator(){


	}//end of deconstructor
	void moveUpInLine(){
		if(money >= 700){
			money -= 600;
			bribed = true;
		}
	}//end of move up in line

	void setAppAccepted(bool b){
		applicationAccepted = b;
	}

	void setPictureTaken(bool b){
		pictureTaken = b;
	}

	bool isAppAccepted(){
		return applicationAccepted;
	}//end of isappaccepted

	bool isPictureTaken(){
		return pictureTaken;
	}//end of of is picture taken

	bool alreadyBribed(){
		return bribed;
	}//end of br

};//end of senator class


<<<<<<< HEAD
class PassPortMonitor {
=======

class PictureMonitor {
>>>>>>> 6cf6ebd943932e264071d895ba47cdae574d075b
private:
	Lock *clerkLineLock;
	//Condition clerkLineCV[5];
	//Condition clerkBribeLineCV[5];
<<<<<<< HEAD
=======

	int clerkLineCount[5];
	int clerkBribeLineCount[5];
	int clerkState[5];	//0: available     1: busy    2: on break

public:
	PictureMonitor(){

	}//end of constructor

	~PictureMonitor(){

	}//end of deconstructor

	Lock* getLock(){
		return clerkLineLock;
	}//end of getting lock
};

struct PassportMonitor {
>>>>>>> 6cf6ebd943932e264071d895ba47cdae574d075b

	Lock* MonitorLock;

	int numClerks;
	Lock** clerkLineLocks;					// move to be global variable
	Condition** clerkLineCV;
	Condition** clerkBribeLineCV;

	int* clerkLineCount;
	int* clerkBribeLineCount;
	int* clerkState;	//0: available     1: busy    2: on break
	std::queue<int>* clientSSNs;
	std::queue<int>* clientReqs; //0: neither picture nor application, 1: 1 of the two, 2: both

	PassportMonitor(int numPassportClerks, int numCustomers)
	{
		MonitorLock = new Lock("Monitor Lock");

		numClerks = numPassportClerks;
		clerkLineLocks = new Lock*[numClerks];
		clerkLineCV = new Condition*[numClerks];
		clerkBribeLineCV = new Condition*[numClerks];
		
		clerkLineCount = new int[numClerks];
		clerkBribeLineCount = new int[numClerks];
		clerkState = new int[numClerks];

		clientSSNs = new std::queue<int>[numCustomers];
		clientReqs = new std::queue<int>[numCustomers];

		for(int i = 0; i < numClerks; i++)
		{			
			clerkLineCV[i] = new Condition("");
			clerkBribeLineCV[i] = new Condition("");
			clerkLineLocks[i] = new Lock("ClerkLineLock");
			clerkLineCount[i] = 0;
			clerkBribeLineCount[i] = 0;
			clerkState[i] = 0;
		}
	}//end of constructor

	~PassportMonitor(){

	}//end of destructor	

	int getSmallestLine()
	{
		int smallest = 50;
		int smallestIndex = 0;
		for(int i = 0; i < numClerks; i++)
		{
			//std::cout << clerkLineCount[i] << std::endl;
			if(clerkLineCount[i] < smallest)
			{
				smallest = clerkLineCount[i];
				smallestIndex = i;
			}
		}
		return smallestIndex;
	}

	void giveSSN(int line, int ssn)
	{
		clientSSNs[line].push(ssn);
	}

	void giveReqs(int line, int completed)
	{
		clientReqs[line].push(completed);
	}

};

class CashierMonitor {
private:
	Lock *clerkLineLock;
	//Condition clerkLineCV[5];
	//Condition clerkBribeLineCV[5];

	int clerkLineCount[5];
	int clerkBribeLineCount[5];
	int clerkState[5];	//0: available     1: busy    2: on break

public:
	CashierMonitor(){

	}//end of constructor

	~CashierMonitor(){

	}//end of deconstructor

	Lock* getLock(){
		return clerkLineLock;
	}//end of getting lock
};




void createCustomer(){
	int rdmMoneyIndex = rand()%4;
	ssnCount++; //Important: ssnCount has to be incremented before run is called. I recommend doing that with other calls below, too.
	std::cout << "rdmMoneyIndex: " << rdmMoneyIndex << std::endl;
	Client *c = new Client(ssnCount, clientStartMoney[rdmMoneyIndex]);	
    c->setselfIndex(customers.size());
    customers.push_back(c);
}//end of making customer

void createApplicationClerk(){
    ApplicationClerk *ac = new ApplicationClerk();
    ac->setselfIndex(customers.size());
    aClerks.push_back(ac);

}//end of making application clerk

void createPassPortClerk(){
    PassPortClerk *ppc = new PassPortClerk();
    ppc->setselfIndex(customers.size());
    ppClerks.push_back(ppc);
}//end of making passportClerk


void createPictureClerk(){
    PictureClerk *pc = new PictureClerk();
    pc->setselfIndex(customers.size());
    pClerks.push_back(pc);


}//end of making picture clerk

void createCashierClerk(){
    Cashier *cc = new Cashier();
    cc->setselfIndex(customers.size());
    cClerks.push_back(cc);


}//end of making cashier clerk

void makeManager(){
    Manager *m = new Manager();

}//end of making manager

void makeSenator(){
    int rdmMoneyIndex = rand()%4;
    std::cout << "rdmMoneyIndex: " << rdmMoneyIndex << std::endl;
    Senator *s = new Senator(ssnCount, clientStartMoney[rdmMoneyIndex]);
    ssnCount++;

}//end of making senator


void Problem2(){
	//srand(time(NULL));
	int customer_thread_num;
	int applicationClerk_thread_num;
	int pictureClerk_thread_num;
	int passPortClerk_thread_num;
	int cashierClerk_thread_num;
	int manager_thread_num = 1; //There can only be one manager in the simulation
	int senator_thread_num;


	bool acceptInput = false;
	int num_of_people = 0;
	//create menu here to figure out how many threads of each
	std::cout << "reached" << std::endl;

	while(!acceptInput){
		std::cout << "Menu :: How many customers? (20 - 50)" << std::endl;
		std::cout << "Input: " << std::endl;
		std::cin >> num_of_people;  
		//num_of_people = checkInput(input, 20, 50);
		if(!std::cin.fail()){
			if(num_of_people >= 20 && num_of_people <= 50){
				customer_thread_num = num_of_people;
				acceptInput = true;
			}
		}//end of if
		else{
			std::cout << " >> Error!  Input not accepted. " << std::endl;
		}
	}//end of while

	acceptInput = false;
	while(!acceptInput){
		std::cout << "Menu :: How many Application Clerks? (1 - 5)" << std::endl;
		std::cout << "Input: " << std::endl;
		std::cin >> num_of_people;  
		//num_of_people = checkInput(input, 1, 5);
		if(!std::cin.fail()){
			if(num_of_people >= 1 && num_of_people <= 5){
				applicationClerk_thread_num = num_of_people;
				acceptInput = true;
			}//end of if
		}//end of if	
		else{
			std::cout << " >> Error!  Input not accepted.  " << std::endl;
		}
	}//end of while

	acceptInput = false;
	while(!acceptInput){
		std::cout << "Menu :: How many Picture Clerks? (1 - 5)" << std::endl;
		std::cout << "Input: " << std::endl;
		std::cin >> num_of_people;  
		//num_of_people = checkInput(input, 1, 5);
		if(!std::cin.fail()){
			if(num_of_people >= 1 && num_of_people <= 5){
				pictureClerk_thread_num = num_of_people;
				acceptInput = true;
			}//end of if
		}//end of if	
		else{
			std::cout << " >> Error!  Input not accepted.  " << std::endl;
		}
	}//end of while

	acceptInput = false;
	while(!acceptInput){
		std::cout << "Menu :: How many PassPort Clerks? (1 - 5)" << std::endl;
		std::cout << "Input: " << std::endl;
		std::cin >> num_of_people;  
		//num_of_people = checkInput(input, 1, 5);
		if(!std::cin.fail()){
			if(num_of_people >= 1 && num_of_people <= 5){
				passPortClerk_thread_num = num_of_people;
				acceptInput = true;
			}//end of if
		}//end of if	
		else{
			std::cout << " >> Error!  Input not accepted.  " << std::endl;
		}
	}//end of while

	acceptInput = false;
	while(!acceptInput){
		std::cout << "Menu :: How many Cashier Clerks? (1 - 5)" << std::endl;
		std::cout << "Input: " << std::endl;
		std::cin >> num_of_people;  
		//num_of_people = checkInput(input, 1, 5);
		if(!std::cin.fail()){
			if(num_of_people >= 1 && num_of_people <= 5){
				cashierClerk_thread_num = num_of_people;
				acceptInput = true;
			}//end of if
		}//end of if	
		else{
			std::cout << " >> Error!  Input not accepted.  " << std::endl;
		}
	}//end of while

     acceptInput = false;
    while(!acceptInput){
        std::cout << "Menu :: How many Senators? (1 - 10)" << std::endl;
        std::cout << "Input: " << std::endl;
        std::cin >> num_of_people;  
        //num_of_people = checkInput(input, 1, 5);
        if(!std::cin.fail()){
            if(num_of_people >= 1 && num_of_people <= 10){
                senator_thread_num = num_of_people;
                acceptInput = true;
            }//end of if
        }//end of if    
        else{
            std::cout << " >> Error!  Input not accepted.  " << std::endl;
        }
    }//end of while

	AMonitor = new ApplicationMonitor(applicationClerk_thread_num, customer_thread_num);
	PMonitor = new PictureMonitor(pictureClerk_thread_num, customer_thread_num);

	//create for loop for each and fork
	//create 
	std::cout << "reached.  customer_thread_num: " << customer_thread_num << std::endl; 
	for(int i = 0; i < customer_thread_num; i++){
		Thread *t = new Thread("customer thread");			
		t->Fork((VoidFunctionPtr)createCustomer, i+1);
		
	}//end of creating client threads

	std::cout << "reached.  applicationClerk_thread_num: " << applicationClerk_thread_num << std::endl; 
	for(int i = 0; i < applicationClerk_thread_num; i++){
		Thread *t = new Thread("application clerk thread");
		t->Fork((VoidFunctionPtr)createApplicationClerk, i+1);
	}//end of creating application clerk threads

    std::cout << "reached.  passPortClerk_thread_num: " << passPortClerk_thread_num << std::endl; 
    for(int i = 0; i < passPortClerk_thread_num; i++){
        Thread *t = new Thread("passport clerk thread");
        t->Fork((VoidFunctionPtr)createPassPortClerk, i+1);
    }//end of creating passPort clerk threads

    std::cout << "reached.  pictureClerk_thread_num: " << pictureClerk_thread_num << std::endl; 
    for(int i = 0; i < pictureClerk_thread_num; i++){
        Thread *t = new Thread("picture clerk thread");
        t->Fork((VoidFunctionPtr)createPictureClerk, i+1);
    }//end of creating picture clerk threads

    std::cout << "reached.  cachierClerk_thread_num: " << cashierClerk_thread_num << std::endl; 
    for(int i = 0; i < cashierClerk_thread_num; i++){
        Thread *t = new Thread("cashier clerk thread");
        t->Fork((VoidFunctionPtr)createCashierClerk, i+1);
    }//end of creating cashier clerk threads

    std::cout <<"reached. manager_thread_num: " << manager_thread_num << std::endl;
    for (int i=0; i<manager_thread_num; i++){
        Thread *t = new Thread("manager thread");
        t->Fork((VoidFunctionPtr)makeManager, i+1);
    }  //end of creating solo manager thread

    std::cout <<"reached. senator_thread_num: " << senator_thread_num << std::endl;
    for (int i=0; i<senator_thread_num; i++){
        Thread *t = new Thread("senator thread");
        t->Fork((VoidFunctionPtr)makeSenator, i+1);
    }  //end of creating senator threads


}//end of problem 2
