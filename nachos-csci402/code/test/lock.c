#include "../userprog/syscall.h"
#define NUM_LOCKS 50
/*Global Var above defining number of locks created */

/*Test functions for create destroy acquire release*/
void create();
void destroy(int create);
//void acquire();
//void release();


int create() {
	Write("---Lock Create Test---\n", 25, ConsoleOutput);
	int lockList[NUM_LOCKS];
	int lockID; //ID defining the lock
	int n; //Size of list

	lockList[0] = CreateLock("Lock #1", 32);

	//Test 1, Determine that List Has Incremented
	n = sizeof(lockList) / sizeof(int);
	if (n > 0) {
		Write("-Successfully added a lock \n", 32, ConsoleOutput);
	}

	else {
		Write("-FAILURE: Could not add lock\n", 32, ConsoleOutput);
		return 0;
	}

}

void destroy (int create){
	Write("---Lock Destroy Test---\n", 30, ConsoleOutput);
	int lockList[NUM_LOCKS];
	int lockID; //ID defining the lock
	int n; //Size of list

	if (create == 0)
	{
		Write("-FAILURE: Cannot Destroy Lock if Create Does Not Work\n", 50, ConsoleOutput);
	}
	else {
		lockList[0] = CreateLock("Lock #1", 50);
		lockList[1] = CreateLock("Lock #2", 31);

		DestroyLock(lockList[0]);

		n = sizeof(lockList) / sizeof(int);

		if (lockList[0] == 1) {
			Write("-Successfully destroyed lock\n", 50, ConsoleOutput);
		}

		else if (lockList[0] == 2) {
			Write("-FAILURE: Cannot Destroy Lock\n", 50, ConsoleOutput);
		}

	}

}



int main() {
	create();
	destroy(create());
	//acquire();
	//release();
	Exit(0);
}