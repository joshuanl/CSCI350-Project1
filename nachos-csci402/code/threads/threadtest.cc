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

//PROTOTYPES
class Client;
class ApplicationClerk;
class PictureClerk;
class PassPortClerk;
class Cashier;	 		

// GLOBAL VARIABLES FOR PROBLEM 2
int ssnCount = 0;
const int clientStartMoney[4] = {100, 500, 1100, 1600};


std::vector<ApplicationClerk *> aClerks;
std::vector<PictureClerk *> pClerks;
std::vector<PassPortClerk *> ppClerks;
std::vector<Cashier *> cClerk;

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


class Client {

private:
	int money;
	int ssn;
	bool applicationAccepted;
	bool pictureTaken;
	bool bribed; 	//reset after each line
public:
	Client(int num, int startMoney){

		ssn = num;
		money = startMoney;
		std::cout << "ssn: " << ssn << "  startMoney: " << startMoney << std::endl;

		applicationAccepted = false;
		pictureTaken = false;
		bribed = false;
	}//end of client constructor

	~Client(){

	}//end of client deconstructor

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

public:
	ApplicationClerk(){
		clerkState = 0;
		lineCount = 0;
		bribeLineCount = 0;
		clerkMoney = 0;
	}//end of constructor

	~ApplicationClerk(){

	}//endo of deconstructor

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
}; //end of class


class PictureClerk {
private:
	int clerkState; // 0: available     1: busy       2: on break
	int lineCount;   
	int bribeLineCount;
	int clerkMoney; //How much money the clerk has
public:

	PictureClerk(){
		clerkState = 0;
		lineCount = 0;
		bribeLineCount = 0;
		clerkMoney = 0;
	}//end of constructor

	~PictureClerk(){

	}//end of deconstructor

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

}; //end of picture clerk

class PassPortClerk {
private:
	int clerkState; // 0: available     1: busy       2: on break
	int lineCount;   
	int bribeLineCount;
	int clerkMoney; //How much money the clerk has
public:

	PassPortClerk(){
		clerkState = 0;
		lineCount = 0;
		bribeLineCount = 0;
		clerkMoney = 0;
	}//end of constructor

	~PassPortClerk(){

	}//end of deconstructor

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


class Cashier {
private:
	int clerkState; // 0: available     1: busy       2: on break
	int lineCount;   
	int bribeLineCount;
	int clerkMoney; //How much money the clerk has

	//place map in monitor
	//map<*Client, bool> clientRecords;   //records client and bool  if a passport got handed back
									  //only one passport per client
public:

	Cashier(){
		clerkState = 0;
		lineCount = 0;
		bribeLineCount = 0;
		clerkMoney=0;
	}//end of constructor

	~Cashier(){

	}//end of deconstructor

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

	// place below in monitor
	// void recordCustomer(Client *c){	//should only be called after payment
	// 	clientRecords(std::pair<*Client, bool>(c, true));
	// }//end of recording customer in books 

}; // end of cashier clerk	


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
			pClerkMoney += aClerks[i]->getclerkMoney();
		}//end of for
		for(unsigned int i=0; i < ppClerks.size(); i++){
			ppClerkMoney += aClerks[i]->getclerkMoney();
		}//end of for
		for(unsigned int i=0; i < cClerk.size(); i++){
			cClerkMoney += aClerks[i]->getclerkMoney();
		}//end of for
		totalMoney = pClerkMoney + aClerkMoney + ppClerkMoney + cClerkMoney;
	}//end of updating total money 

    int getaClerkMoney() {
        return aClerkMoney;
    }
    
    int getpClerkMoney() {
        return pClerkMoney;
    }
    
    int getppClerkMoney() {
        return ppClerkMoney;
    }
    
    int getcClerkMoney() {
        return cClerkMoney;
    }
    
    int gettotalMoney() {
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

	Senator(){

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

class ApplicationMonitor {
private:
	Lock *clerkLineLock;					// move to be global variable
	//Condition clerkLineCV[5];
	//Condition clerkBribeLineCV[5];

	int clerkLineCount[5];
	int clerkBribeLineCount[5];
	int clerkState[5];	//0: available     1: busy    2: on break

public:
	ApplicationMonitor(){

	}//end of constructor

	~ApplicationMonitor(){

	}//end of deconstructor

	Lock* getLock(){
		return clerkLineLock;
	}//end of getting lock
};

class PictureMonitor {
private:
	Lock *clerkLineLock;
	//Condition clerkLineCV[5];
	//Condition clerkBribeLineCV[5];

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

class PassPortMonitor {
private:
	Lock *clerkLineLock;
	//Condition clerkLineCV[5];
	//Condition clerkBribeLineCV[5];

	int clerkLineCount[5];
	int clerkBribeLineCount[5];
	int clerkState[5];	//0: available     1: busy    2: on break

public:
	PassPortMonitor(){

	}//end of constructor

	~PassPortMonitor(){

	}//end of deconstructor

	Lock* getLock(){
		return clerkLineLock;
	}//end of getting lock
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




void runCustomer(){
	int rdmMoneyIndex = rand()%4;
	std::cout << "rdmMoneyIndex: " << rdmMoneyIndex << std::endl;
	Client *c = new Client(ssnCount, clientStartMoney[rdmMoneyIndex]);
	ssnCount++;


}//end of making customer

void applicationClerk(){
    ApplicationClerk *ac = new ApplicationClerk();
    aClerks.insert(ac);

}//end of making application clerk

void passPortClerk(){
    PassPortClerk *ppc = new PassPortClerk();
    ppClerks.insert(ppc);
}//end of making passportClerk


void pictureClerk(){
    PictureClerk *pc = new PictureClerk();
    pClerks.insert(pc);


}//end of making picture clerk

void makeManager(){
    Manager *m = new Manager();

}//end of making manager

void makeSenator(){
    int rdmMoneyIndex = rand()%4;
    std::cout << "rdmMoneyIndex: " << rdmMoneyIndex << std:endl;
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
	int manager_thread_num;
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
		std::cout << "Menu :: How many Picture Clerks? (1 - 5)" << std::endl;
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

	//create for loop for each and fork
	//create 
	std::cout << "reached.  customer_thread_num: " << customer_thread_num << std::endl; 
	for(int i = 0; i < customer_thread_num; i++){
		Thread *t = new Thread("customer thread");			
		t->Fork((VoidFunctionPtr)runCustomer, i+1);
		
	}//end of for

	std::cout << "reached.  applicationClerk_thread_num: " << applicationClerk_thread_num << std::endl; 
	for(int i = 0; i < applicationClerk_thread_num; i++){
		Thread *t = new Thread("application clerk thread");
		t->Fork((VoidFunctionPtr)applicationClerk, i+1);
	}//end of for

}//end of problem 2
