# CSCI350-Project1
Title: Writeup for Project 1 Fall 2015
Date: 09/19/2015

## I. Requirements
Part I: We are to implement locks and condition variables. We have the option to either user Semaphores or a more primitive method such as Thread::Sleep in the implementation. The implementation should address issues such as the lock not holding and other possibilities that violate the rule of mutual exclusion. The implementation must enforce the concept that only the thread that acquired the lock may release the lock. We are to ignore the instructions that if a thread is to do something illegal, that we must terminate Nachos. Make sure that the code is commented, and test the code running the test_code.cc.

Part 2: We are to create a simulation of the U.S. Passport Office that utilizes different threads for all of the staff and customers in the simulation. The simulation includes the following people and resources:
* Customer
* Application Clerk
* Picture Clerk
* Passport Clerk
* Cashier
* Manager

Each one of these people is responsible for a distinct set of actions and interactions with one another. Likewise, we are to keep track of items such as sales, and the sales from each clerk. The particular actions that we are to test include:
* Customers always take the shortest line, but no 2 customers ever choose the same shortest line at the same time
* Managers only read one from one Clerk's total money received, at a time.
* Customers do not leave until they are given their passport by the Cashier. The Cashier does not start on another customer until they know that the last Customer has left their area
* Clerks go on break when they have no one waiting in their line
* Managers get Clerks off their break when lines get too long
* Total sales never suffers from a race condition
* The behavior of Customers is proper when Senators arrive. This is before, during, and after. 

The simulation is to support the following numbers of each type of person:

* Customers: up to 50
* Clerks and Cashier: between 1 and 5
* Manager: only 1
* Senators: up to 10

## II. Assumptions
Part 2:
* We assume that if multiple clerks are on break, and multiple lines reach more than 3 customers waiting in line, the manager is to wake up one clerk at a time at an order chosen by us (can be random).
* We assume that 5% of the time, the Passport Clerk will make a mistake with a customer and will not be able to process their application and the customer is to return to the back of the line.
* Instead of using a data structure such as a list or an array, we use a Boolean to denote if a customer's application has been accepted, denoted by the Boolean 'isAppAccepted'.
* We are not to use hash tables, because in the next project, Project 2, we will be converting this project to a Nachos user program written with only C, and further limitations. We can use ints and chars, and int arrays and char arrays- as well as structs of ints and chars. 
* In the scenario with a limited number of customers, the clerk going on break scenario may result in breakage. For instance, if there is only one customer and a clerk goes on break- the customer will be standing in line eternally as more than three customers are required to be in line for the manager to bring the clerk back from the break. We do not have to address this particular item of breakage, it is allowed by this assignment.
* Each interaction between two threads should have a different lock, we should not be holding onto a lock for a long time. It is assumed that we use different values of the 'rs' parameter to get context switches in different places.
* The Manager class is a thread like the rest of the personelle in the simulation. In order to create timer capacity, we should use the Thread->Yield().
* 20-50 customer threads allowed in the Big System test. For personal testing, we can use any number of customer threads.
* All clerks can be bribed, but the cashier cannot be bribed.
* Make a form of user input to be able to set the number of clerks prior to running the simulation.

## III. Design
Part 2: A description of Classes added and Methods.
* Class Client (aka Customer in Simulation) contains:
  * The following variables: the int money representing how much money the customer has and int ssn to represent the social security number and customer identifier. The class contains two booleans to represent the stages of going through the passport office: pictureTaken to indicate whether the customer has successfully interacted with the Picture Clerk and received a photo, and applicationAccepted to denote whether or not the customer's application has been accepted after interacting with the Application clerk. There is an additional Boolean, bribed, that keeps track of whether the customer has currently bribed during this line, or not. The bribe boolean is reset with each line that the customer is in. 
  * The following methods: moveUpInLine(), which will allow a customer with greater than or equal to 700 dollars to change their bribed variable to true for the following line that they are in, and result in a net loss of 600 dollars for the customer, setAppAccepted()- a method allowing the bool applicationAccepted to be set, setPictureTaken()- a method allowing the bool pictureTaken to be set, and the bool methods isAppAccepted(), isPictureTaken(), and alreadyBribed() which returns the booleans set in the Client class.

* Class Application Clerk
  * The following variables: the int clerkState which indicates the clerks state as follows: 0 available, 1 busy, 2 on break, the int lineCount representing the total number of customers in the clerk's line, and the int bribeCount, indicating the number of customers who have bribed the clerk waiting in line.
  * The following methods: a getter and setter method for clerkState, denoted as getclerkState() and setclerkState(), the method addToLine() which increases lineCount by 1, the method addToBribeLine() which increases the bribeCount by 1, the method goToWork() which changes the clerk's state to 1 (busy), and the method goOneBreak() which changes the clerk's state to 2(busy).

* Class Picture Clerk
  * The following variables: the int clerkState which indicates the clerks state as follows: 0 available, 1 busy, 2 on break, the int lineCount representing the total number of customers in the clerk's line, and the int bribeCount, indicating the number of customers who have bribed the clerk waiting in line.
  * The following methods: a getter and setter method for clerkState, denoted as getclerkState() and setclerkState(), the method addToLine() which increases lineCount by 1, the method addToBribeLine() which increases the bribeCount by 1, the method goToWork() which changes the clerk's state to 1 (busy), and the method goOneBreak() which changes the clerk's state to 2(busy).
 
* Class Passport Clerk
  * The following variables: the int clerkState which indicates the clerks state as follows: 0 available, 1 busy, 2 on break, the int lineCount representing the total number of customers in the clerk's line, and the int bribeCount, indicating the number of customers who have bribed the clerk waiting in line.
  * The following methods: a getter and setter method for clerkState, denoted as getclerkState() and setclerkState(), the method addToLine() which increases lineCount by 1, the method addToBribeLine() which increases the bribeCount by 1, the method goToWork() which changes the clerk's state to 1 (busy), and the method goOneBreak() which changes the clerk's state to 2(busy).

* Class Cashier
  * The following variables: The int clerkState which indicates the clerks state as follows: 0 available, 1 busy, 2 on break, the int lineCount representing the total number of customers in the clerk's line, and the int bribeCount, indicating the number of customers who have bribed the clerk waiting in line.
  * The following methods: A getter and setter method for clerkState, denoted as getclerkState() and setclerkState(), the method addToLine() which increases lineCount by 1, the method addToBribeLine() which increases the bribeCount by 1, the method goToWork() which changes the clerk's state to 1 (busy), and the method goOneBreak() which changes the clerk's state to 2(busy).

* Class Manager
  * The following variables: A vector aClerks keeping track of all of the application clerks, a vector pClerks keeping track of all the picture clerks, a vector ppClerks keeping track of all of the passport clerks, and a vector cClerks keeping track of all of the cashiers. An int aMoney keeping track of all of the application clerk's money, an int pMoney keeping track of all of the picture clerk's money, an int ppMoney keeping track of all the passport clerk's money, an int cMoney keeping track of all of the cashier's money, and an int called totalMoney which is a sum of all of the clerks' money.
  * The following methods: A getter and setter method for all of the clerks' respective money totals(getaMoney(),setaMoney(int), getpMoney(),setpMoney(int), getppMoney(), setppMoney(int), get cMoney(), setcMoney(int), gettotalMoney(), and settotalMoney(int, int, int, int), and a wakeClerk(Clerk) method that will change the state of a clerk currently on break because there are greater than three people waiting in their line.