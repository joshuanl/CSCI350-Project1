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

## III. Design
