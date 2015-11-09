// exception.cc 
//  Entry point into the Nachos kernel from user programs.
//  There are two kinds of things that can cause control to
//  transfer back to here from user code:
//
//  syscall -- The user code explicitly requests to call a procedure
//  in the Nachos kernel.  Right now, the only function we support is
//  "Halt".
//
//  exceptions -- The user code does something that the CPU can't handle.
//  For instance, accessing memory that doesn't exist, arithmetic errors,
//  etc.  
//
//  Interrupts (which can also cause control to transfer from user
//  code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include <stdio.h>
#include <iostream>
#include "synch.h"
#include "addrspace.h"


using namespace std;

extern KernelLock* kernelLocks[];
extern KernelCV* kernelCVs[];
extern SingleProcess processTable[20];
extern int processTableCount;
extern Lock* processTableLock;
extern int MVTable[200];
int nLockIndex = 0;
int num_MVs = 0;
int num_CVs = 0;
Lock* editCVsLock = new Lock("editCVsLock");
Lock* tlbLock = new Lock("Tlb Lock");
Lock* mvLock = new Lock("MV Lock");


int copyin(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes from the current thread's virtual address vaddr.
    // Return the number of bytes so read, or -1 if an error occors.
    // Errors can generally mean a bad virtual address was passed in.
    bool result;
    int n=0;            // The number of bytes copied in
    int *paddr = new int;

    while ( n >= 0 && n < len) {
      result = machine->ReadMem( vaddr, 1, paddr );
      while(!result) // FALL 09 CHANGES
      {
            result = machine->ReadMem( vaddr, 1, paddr ); // FALL 09 CHANGES: TO HANDLE PAGE FAULT IN THE ReadMem SYS CALL
      } 
      
      buf[n++] = *paddr;
     
      if ( !result ) {
    //translation failed
    return -1;
      }

      vaddr++;
    }

    delete paddr;
    return len;
}

int copyout(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes to the current thread's virtual address vaddr.
    // Return the number of bytes so written, or -1 if an error
    // occors.  Errors can generally mean a bad virtual address was
    // passed in.
    bool result;
    int n=0;            // The number of bytes copied in

    while ( n >= 0 && n < len) {
      // Note that we check every byte's address
      result = machine->WriteMem( vaddr, 1, (int)(buf[n++]) );

      if ( !result ) {
    //translation failed
    return -1;
      }

      vaddr++;
    }

    return n;
}

void Create_Syscall(unsigned int vaddr, int len) {
    // Create the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  No
    // way to return errors, though...
    char *buf = new char[len+1];    // Kernel buffer to put the name in

    if (!buf) return;

    if( copyin(vaddr,len,buf) == -1 ) {
    printf("\t%s","Bad pointer passed to Create\n");
    delete buf;
    return;
    }

    buf[len]='\0';

    fileSystem->Create(buf,0);
    delete[] buf;
    return;
}

int Open_Syscall(unsigned int vaddr, int len) {
    // Open the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  If
    // the file is opened successfully, it is put in the address
    // space's file table and an id returned that can find the file
    // later.  If there are any errors, -1 is returned.
    char *buf = new char[len+1];    // Kernel buffer to put the name in
    OpenFile *f;            // The new open file
    int id;             // The openfile id

    if (!buf) {
    printf("\t%s","Can't allocate kernel buffer in Open\n");
    return -1;
    }

    if( copyin(vaddr,len,buf) == -1 ) {
    printf("\t%s","Bad pointer passed to Open\n");
    delete[] buf;
    return -1;
    }

    buf[len]='\0';

    f = fileSystem->Open(buf);
    delete[] buf;

    if ( f ) {
    if ((id = currentThread->space->fileTable.Put(f)) == -1 )
        delete f;
    return id;
    }
    else
    return -1;
}

void Write_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one. For disk files, the file is looked
    // up in the current address space's open file table and used as
    // the target of the write.
    
    char *buf;      // Kernel buffer for output
    OpenFile *f;    // Open file for output

    if ( id == ConsoleInput) return;
    
    if ( !(buf = new char[len]) ) {
    printf("\t%s","Error allocating kernel buffer for write!\n");
    return;
    } else {
        if ( copyin(vaddr,len,buf) == -1 ) {
        printf("\t%s","Bad pointer passed to to write: data not written\n");
        delete[] buf;
        return;
    }
    }

    if ( id == ConsoleOutput) {
      for (int ii=0; ii<len; ii++) {
    printf("%c",buf[ii]);
      }

    } else {
    if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
        f->Write(buf, len);
    } else {
        printf("\t%s","Bad OpenFileId passed to Write\n");
        len = -1;
    }
    }

    delete[] buf;
}

int Read_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one.    We reuse len as the number of bytes
    // read, which is an unnessecary savings of space.
    char *buf;      // Kernel buffer for input
    OpenFile *f;    // Open file for output

    if ( id == ConsoleOutput) return -1;
    
    if ( !(buf = new char[len]) ) {
    printf("\t%s","Error allocating kernel buffer in Read\n");
    return -1;
    }

    if ( id == ConsoleInput) {
      //Reading from the keyboard
      scanf("%s", buf);

      if ( copyout(vaddr, len, buf) == -1 ) {
    printf("\t%s","Bad pointer passed to Read: data not copied\n");
      }
    } else {
    if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
        len = f->Read(buf, len);
        if ( len > 0 ) {
            //Read something from the file. Put into user's address space
            if ( copyout(vaddr, len, buf) == -1 ) {
            printf("\t%s","Bad pointer passed to Read: data not copied\n");
        }
        }
    } else {
        printf("\t%s","Bad OpenFileId passed to Read\n");
        len = -1;
    }
    }

    delete[] buf;
    return len;
}

void Close_Syscall(int fd) {
    // Close the file associated with id fd.  No error reporting.
    OpenFile *f = (OpenFile *) currentThread->space->fileTable.Remove(fd);

    if ( f ) {
      delete f;
    } else {
      printf("\t%s","Tried to close an unopen file\n");
    }
}

void Yield_Syscall() {
    currentThread->Yield();
   // printf("\tin yield syscall\n");
    //in test directory, put yield call testfile.c on top of main
    //gmake in test, cd to userprog, do a gmake to compile nachos, nachos
}
//returns int
int CreateLock_Syscall(unsigned int vaddr, int len) {
    // name of lock is in the  buffer pointed to by vaddr
    char *buf = new char[len+1];    // Kernel buffer to put the name in
   // sprintf(b\tuf);
   // if (!buf) return; //check for null value
   // if(size <= 0 || size > NUM){
         //buffer overflow check (negative indeces, very big number)
     //   return -1;
   // }else if(name == NULL){
         //null addresscheck
     //   return -1;
   /// }else if(//illegal address check){
      //  return -1;
    //}
    
	if(vaddr == NULL) {
		printf("\tCheck vaddr parameter in CreateLock_Syscall");
		return -1;
	}
	
	if((vaddr < 0) || (vaddr > ((currentThread -> space -> numPages) * PageSize))) {
		printf("\tCheck if vaddr parameter is within bounds in CreateLock_Syscall");
		return -1;
	}
	
	if(copyin(vaddr, len, buf) == -1) {
		printf("\tCheck pointer passed into CreateLock_Syscall");
		delete buf;
		return -1;
	}
    
    if(!buf) {
    	printf("\tCheck len parameter in CreateLock_Syscall");
    	return -1;
    }
    Lock* createLockLock = new Lock("createLockLock");
    createLockLock->Acquire();
    printf("\tCreated Lock with Index: %d\n", nLockIndex);
  
    //create lock object
    KernelLock* newLock = new KernelLock;
    newLock->lock = new Lock(buf);
    newLock->isToBeDeleted = false;
    newLock->addrSpace = currentThread->space;
     //put in a table @ some index
    kernelLocks[nLockIndex] = newLock;
    nLockIndex++;
    int returnLockIndex = nLockIndex-1;
    //printf("\tDecimals: %d\n", nLockIndex);
    createLockLock->Release();
    printf("returnLockIndex: %d\n", returnLockIndex);
    //return kernel lock table index to user program
    return returnLockIndex;
}


void Acquire_Syscall(int id){
    if (id < 0 || id > 200){
        printf("\tAcquire invalid lock index, out of bound...\n");
        return;
    }
    printf("\tAcquiring the lock with Index: %d\n", id);
    //write validation if neg value or exceeds maximum locks
    kernelLocks[id]->lock->Acquire();
    //cant test mutual exclusion until i have two threads through fork system call....i can call it but its only one thread....wont put thread to sleep
}
//condition->Wait i can writeSystemcall should not be called.
//wait()
//writer();

void Release_Syscall(int id){
    if (id < 0 || id > 200){
        printf("\tReleaseing a lock that is out of bound...\n");
        return;
    }
    
    if (kernelLocks[id] == NULL){
        printf("\tReleaseing a already destroyed lock...\n");
        return;
    }
    if (kernelLocks[id]->lock == NULL){
        printf("\tReleaseing a nonexistent lock...\n");
        return;
    }
    printf("\tReleaseing a lock with index: %d\n", id);
    kernelLocks[id]->lock->Release();
}

void DestroyLock_Syscall(int id){
    //check if wrong index bounds
    
    if(kernelLocks[id]->lock == NULL) {
    	printf("\tCheck if lock exists – cannot destroy nonexistent lock");
    	return;
    }
    
    if(id > nLockIndex || id < 0) {
    	printf("\tCheck if lock index exists – cannot destroy lock out of bounds");
    	return;
    }
    printf("\tDestroying a lock with index: %d\n", id);
    kernelLocks[id]->isToBeDeleted = true;
    delete kernelLocks[id]->lock;
    delete kernelLocks[id];
    kernelLocks[id] = NULL;
    //destroy and acquire after...should get an error, use an invalid index
}

//Condition variable syscalls
int CreateCondition_Syscall(unsigned int vaddr, int len) {
	
	if(vaddr == NULL) {
		printf("\tCheck vaddr parameter in CreateCondition_Syscall\n");
		return -1;
	}
	
	if((vaddr < 0) || (vaddr > ((currentThread -> space -> numPages) * PageSize))) {
		printf("\tCheck if vaddr parameter is within bounds in CreateCondition_Syscall\n");
		return -1;
	}
	
	char* name = new char[len + 1];
	
	if(copyin(vaddr, len, name) == -1) {
		printf("\tCheck pointer passed into CreateCondition_Syscall\n");
		delete name;
		return -1;
	}
    
    if(!name) {
    	printf("\tCheck len parameter in CreateCondition_Syscall\n");
    	return -1;
    }

    
    sprintf(name, "Condition %d", vaddr);           //assign unique identifier to this CV
    printf("\tCreated CV with index: %d\n", num_CVs);
    KernelCV* t = new KernelCV;
    t -> cv = new Condition(name);
    t -> isToBeDeleted = false;
    t -> as = currentThread -> space;           //actually create a new CV instance and perform declarations
    
    editCVsLock -> Acquire();                   //begin critical section
    if (num_CVs < 200){
        kernelCVs[num_CVs] = t;                 //add newly constructed CV to list of kernel CVs
        num_CVs++;
    }
    editCVsLock -> Release();                   //end critical section

    return num_CVs-1;
}

void DestroyCondition_Syscall(int id) {     
    
    if(id < 0 || id >= 200){
        printf("\tError: Destroying a CV out of bound..\n");
        return;
    }
    if(kernelCVs[id] == NULL){
        printf("\tError: Destroying an already destroyed CV..\n");
        return;
    }
    if(kernelCVs[id]->cv == NULL){
        printf("\tError: Destroying a nonexistent CV..\n");
        return;
    }
    kernelCVs[id] -> isToBeDeleted = true;  //set intent to destroy this CV
    if((kernelCVs[id] -> isToBeDeleted) && (kernelCVs[id] -> cv -> getWaitingLock() == NULL)) { //note: associated lock must be destroyed too
        printf("Successfully deleted CV with index: %d\n", id);
        delete kernelCVs[id] -> cv;
        delete kernelCVs[id];
        kernelCVs[id] = NULL;
    }

}

void Wait_Syscall(int cvIndex, int lockIndex) { //TO DO: change Lock type to the more protected KernelLock
    if (cvIndex < 0 || cvIndex >= 200){
        printf("\tError: Waiting on a CV out of bound...\n");
        return;
    }
    if (lockIndex < 0 || lockIndex>= 200){
        printf("\tError: Incorrect associated lock...\n");
        return;
    }
    printf("\tWaiting on CV %d, lock %d\n", cvIndex, lockIndex);
    KernelLock* cvLock = kernelLocks[lockIndex];
    kernelCVs[cvIndex] -> cv -> Wait(cvLock->lock); //TO DO: change cvWaitLock parameter to actual lock of KernelLock
}

void Signal_Syscall(int cvIndex, int lockIndex) {   //TO DO: change Lock type to the more protected KernelLock
    if (cvIndex < 0 || cvIndex >= 200){
        printf("\tError: Signalling a CV out of bound...\n");
        return;
    }
    if (lockIndex < 0 || lockIndex>= 200){
        printf("\tError: Incorrect associated lock...\n");
        return;
    }
    KernelLock* cvLock = kernelLocks[lockIndex];
    if (cvLock->lock != NULL){
        printf("\tSignalling CV with Index: %d with Lock: %d\n", cvIndex, lockIndex);
        kernelCVs[cvIndex]->cv->Signal(cvLock->lock);
    }
}

void Broadcast_Syscall(int cvIndex, int lockIndex) {    //TO DO: change Lock type to the more protected KernelLock
    //TODO: Need data Validation - NOTE: not needed
    KernelLock* cvLock = kernelLocks[lockIndex];
    kernelCVs[cvIndex]->cv->Broadcast(cvLock->lock);
}

int CreateMV_Syscall(){
    return 0;//for now use 0
}
void DestroyMV_Syscall(int mvIndex){

}
void GetMV_Syscall(int mvIndex){

}
void SetMV_Syscall(int mvIndex, int newIndex){

}


struct ForkThreadInfo{
    int vaddr;
    int pageAddress;
};

void ForkHelper(int threadInfo){
    ForkThreadInfo* tInfo = (ForkThreadInfo*) threadInfo;
    int vaddr = tInfo->vaddr;
    int pageAddress = tInfo->pageAddress;

    //update PCReg to vaddr
    //see save state in addrSpace
    machine->WriteRegister(PCReg, vaddr);
    machine->WriteRegister(NextPCReg, vaddr+4);

    //update StackReg so thread starts from starting point of stackReg
    machine->WriteRegister(StackReg, pageAddress);

    currentThread->space->RestoreState();

    machine->Run();
}

void Fork_Syscall(unsigned int vaddr){
    //data validation
    if(vaddr < 0){
        printf("\tThe address passed in is invalid...\n");
        return;
    }

    //valid address
    //init thread that will be forked
    Thread* newThread = new Thread("new_forked_thread");
    newThread->space = currentThread->space;
    
    //ForkThreadInfo* t;
    //t -> vaddr = vaddr;

    //save current state, PC, and machine registers before fork()
    int threadPageAddr = newThread->space->AllocatePages();
    //will return -1 if no bits are clear
    if (threadPageAddr != -1){
        newThread->firstStackPage = threadPageAddr;

        //create a ForkThreadInfo instance  
        //to store vaddr and pageAddr of newly created thread 
        ForkThreadInfo* threadInfo = new ForkThreadInfo();
        threadInfo->vaddr = vaddr;
        threadInfo->pageAddress = threadPageAddr;

        // update process table
        processTableLock->Acquire();
        for (int i = 0; i < 20; ++i){
            if(processTable[i].as == currentThread->space){
                processTable[i].threads[processTable[i].threadCount] = newThread;
                //t -> pageAddress = processTable[i].threadCount;
                processTable[i].threadCount++;
                break;
            }
        }
        processTableLock->Release();

        //addressSpace of thread done allocating 
        //pageTable done updating
        //now fork newly created thread: newThread
        newThread->Fork(ForkHelper, (int)threadInfo);
        currentThread->Yield();
    }
    //Should't happen...
    else{
        printf("\tno bits are clear in bitmap...\n");
    }

}

struct ExecThreadInfo {
	int vaddr;
	int pageAddress;
	ExecThreadInfo(): pageAddress(-1) {}
};

void exec_thread(int num){
    //initialize the registers by using currentthread->Space
    currentThread->space->InitRegisters();
    
    //Call Restore State through currentThread->Space
    currentThread->space->RestoreState();
    
    //Switch to User Mode
    machine->Run();
    
}

void Exec_Syscall(unsigned int vaddr, int len){
    printf("Calling Exec");
    char *buf = new char[len+1];
    
    if(!buf) {
        printf("Check len parameter in Exec_Syscall");
        return;
    }
    
    //Convert to physical address and read contents
    if(copyin(vaddr, len, buf) == -1) {
        printf("Check pointer passed into Exec_Syscall");
        delete buf;
        return;
    }
    
    //Open the file & store openfile pointer
    OpenFile *executable = fileSystem->Open(buf);
    
    if(executable == NULL){
        printf("Not able to open file for Exec_Syscall");
        return;
    }
    //New process, new address space
    AddrSpace *space;
    
    //AddrSpace arg takes in a filename. New address space for executable file
    //Address space consturtor cannot handle multiple processes.
    space = new AddrSpace(executable);
    
    //Create a new thread in new address space
    Thread* thread = new Thread(buf);
    //allocate the space created to this thread's space
    thread->space = space;
    thread->firstStackPage = thread->space->AllocatePages();
    thread->space->AllocatePages();
    
    
    //Fork new thread
    thread->Fork(exec_thread, 123);
}

void Exit_Syscall(int val){
     printf("number %d", val);
    /* Exit requirements (as per assignment document): 
    The Exit system call must ensure that Thread::Finish is called, except for the very last thread running in Nachos. 
    For the very last thread (not the last thread in a process - unless it's the last process in Nachos), 
    you must call interrupt->Halt() to actually stop Nachos. If you don't do this, Nachos will not terminate. 
    This assignment requires that Nachos terminates.
    */
    //to be deleted
   /*if (true){
        printf("number %d", val);
        printf("Exiting Thread..\n");
        
        currentThread->Finish();
       // return 0;
        return;
    }*/
    //to be deleted
    processTableLock -> Acquire();				//beginning critical section
    int num_proc_with_threads, curr_thread_index = 0;
    for(int i = 0; i < 10; i++) {
    	if(processTable[i].threadCount > 0) {	//if a certain process does have running threads
    		num_proc_with_threads++;
    	}
    }
    
    if(num_proc_with_threads == 1) {
    	interrupt -> Halt();					//last thread in last process - done 
    }
    
    int process_id_done = 0;
    for(int i = 0; i < 10; i++) {
    	if((processTable[i].as == currentThread -> space) && (processTable[i].threadCount == 1)) {	
    		//if current thread is part of a process in the process table 
    		//&& this process has only the one thread
    		//i.e. this is the last thread in a specific process
    		
    		process_id_done = i;
    		
    		for(int j = 0; j < nLockIndex; j++) {
    			if(kernelLocks[j] -> addrSpace == processTable[process_id_done].as) {
    				DestroyLock_Syscall(j);
    				CreateLock_Syscall(j, 16);
    			}
    		}
    		
    		for(int k = 0; k < num_CVs; k++) {
    			if(kernelCVs[k] -> as == processTable[process_id_done].as) {
    				DestroyCondition_Syscall(k);
    				CreateCondition_Syscall(k, 16);
    			}
    		}
    		
    		processTable[i].as -> DeallocatePages();	//deallocate all pages of this process – this process is done
    		
    		currentThread -> Finish();
    		//return 0;
            return;
    	}
    }
    
    for(int i = 0; i < processTable[process_id_done].threadCount; i++) {
    	if(currentThread == processTable[process_id_done].threads[i]) {
    		processTable[process_id_done].as -> DeallocateEightPages(processTable[process_id_done].threads[i] -> firstStackPage);
    		currentThread -> Finish();
    		//return 1;
            return;
    	}
    }
    
    printf("\tExit syscall didn't work");
   // return -1;
    return;
    
    processTableLock -> Release();				//ending critical section
    
}


void PrintInt_Syscall(int num){
    std::cout << "PRINT NUM: " << num << std::endl;
}


void HandlePageFault(int neededVPN){
    
    //disable Interrupts
    IntStatus old = interrupt->SetLevel(IntOff);
    int PPN = -1;
    //cout << "In handlepagefault*****************" << endl;

/*   
    populate tlb with page table (step1)
 
    machine->tlb[tlbIndex].virtualPage = currentThread->space->pageTable[neededVPN].virtualPage;
    machine->tlb[tlbIndex].physicalPage = currentThread->space->pageTable[neededVPN].physicalPage;
    machine->tlb[tlbIndex].valid = currentThread->space->pageTable[neededVPN].valid;
    machine->tlb[tlbIndex].use = currentThread->space->pageTable[neededVPN].use;
    machine->tlb[tlbIndex].dirty = currentThread->space->pageTable[neededVPN].dirty;
    machine->tlb[tlbIndex].readOnly = currentThread->space->pageTable[neededVPN].readOnly;*/
    
    //Look for index (physical page number) in order to search through IPT
    for(int i = 0; i < NumPhysPages; i++){
        //printf("In handlepagefault");
        if(ipt[i].valid == TRUE && ipt[i].virtualPage == neededVPN && ipt[i].as == currentThread->space){
            PPN = i;
            cout << "ipt valid bit= " << ipt[i].valid << "ipt virtual page=" << ipt[i].virtualPage << "ipt as=" << ipt[i].as << endl;
            i += NumPhysPages;
        }

    }
    if(PPN == -1){
        return;
    }
    //populate tlb with ipt
    machine->tlb[tlbIndex].virtualPage = ipt[PPN].virtualPage;
    machine->tlb[tlbIndex].physicalPage = ipt[PPN].physicalPage;
    machine->tlb[tlbIndex].valid = ipt[PPN].valid;
    machine->tlb[tlbIndex].use = ipt[PPN].use;
    machine->tlb[tlbIndex].dirty = ipt[PPN].dirty;
    machine->tlb[tlbIndex].readOnly = ipt[PPN].readOnly;
    
    tlbIndex++;
    tlbIndex = tlbIndex % 4;
    
    //restore interrupts
    interrupt->SetLevel(old);

  
}

void ExceptionHandler(ExceptionType which) {
    int type = machine->ReadRegister(2); // Which syscall?
    int rv=0;   // the return value from a syscall

    if ( which == SyscallException ) {
    switch (type) {
        default:
        DEBUG('a', "Unknown syscall - shutting down.\n");
        case SC_Halt:
        DEBUG('a', "Shutdown, initiated by user program.\n");
        interrupt->Halt();
        break;
        case SC_Create:
        DEBUG('a', "Create syscall.\n");
        Create_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
        break;
        case SC_Open:
        DEBUG('a', "Open syscall.\n");
        rv = Open_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
        break;
        case SC_Write:
        DEBUG('a', "Write syscall.\n");
        Write_Syscall(machine->ReadRegister(4),
                  machine->ReadRegister(5),
                  machine->ReadRegister(6));
        break;
        case SC_Read:
        DEBUG('a', "Read syscall.\n");
        rv = Read_Syscall(machine->ReadRegister(4),
                  machine->ReadRegister(5),
                  machine->ReadRegister(6));
        break;
        case SC_Close:
        DEBUG('a', "Close syscall.\n");
        Close_Syscall(machine->ReadRegister(4));
        break;
        
        case SC_Yield:
        DEBUG('a', "Yield syscall.\n");
        Yield_Syscall();
        break;
            
        case SC_CreateLock:
        DEBUG('a', "CreateLock syscall.\n");
        rv = CreateLock_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
        break;
            
        case SC_DestroyLock:
        DEBUG('a', "DestroyLock syscall.\n");
        DestroyLock_Syscall(machine->ReadRegister(4));
        break;

        case SC_Acquire:
        DEBUG('a', "Acquire syscall.\n");
        Acquire_Syscall(machine->ReadRegister(4));
        break;

        case SC_Release:
        DEBUG('a', "Release syscall.\n");
        Release_Syscall(machine->ReadRegister(4));
        break;

        case SC_CreateCondition:
        DEBUG('a', "CreateCondition syscall.\n");
        rv = CreateCondition_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
        break;

        case SC_DestroyCondition:
        DEBUG('a', "DestroyCondition syscall.\n");
        DestroyCondition_Syscall(machine->ReadRegister(4));
        break;

        case SC_Signal:
        DEBUG('a', "Signal syscall.\n");
        Signal_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
        break;

        case SC_Broadcast:
        DEBUG('a', "Broadcast syscall.\n");
        Broadcast_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
        break;

        case SC_Wait:
        DEBUG('a', "Wait syscall.\n");
        Wait_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
        break;


        case SC_Fork:
        DEBUG('a', "Fork syscall.\n");
        Fork_Syscall(machine->ReadRegister(4));
        break;
        
        case SC_Exec:
        DEBUG('a', "Exec syscall.\n");
        Exec_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
        break;

        case SC_Exit:
        DEBUG('a', "Exit syscall.\n");
        Exit_Syscall(machine->ReadRegister(4));
        break;
            
        case SC_PrintInt:
        DEBUG('a', "Print syscall.\n");
        PrintInt_Syscall(machine->ReadRegister(4));
        break;

    }

    // Put in the return value and increment the PC
    machine->WriteRegister(2,rv);
    machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
    machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
    machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
    return;
        //pagefault exception case
    }else if(which == PageFaultException ){
        //printf("IN PAGEFAULT EXCEPTION!!!*!*!**!\n");
         //cout << "IN PAGEFAULT EXCEPTION****************";
        int neededVPN = machine->ReadRegister(BadVAddrReg)/PageSize;
        //Finds needed Virtual Page& copys information into TLB.
        HandlePageFault(neededVPN);
        // Put in the return value and increment the PC
        //machine->WriteRegister(2,rv);
    
    }else {
      cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
      interrupt->Halt();
    }
}

