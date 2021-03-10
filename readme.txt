This assignment is created by Mathew Lawrence, 17354272

Assignment 2.
Lift simulator.

I chose to use mutex's (mutual exclusion semaphore) in certain areas in the program.

Some notable locations are:
1.When the program chooses to print, it should only do that if the console is not already being used.
2.When an elevator arrives on a floor, nothing should alter the floor or else there might be a race condition.
    A.This means that when the lift says "get into lift" it should have sole access to the floor until it is either full or there are no people on the floor.
    B.To fully implement the mutex on the floor, the waiting to go up and go down need to have a lock on themselves too; We dont want a race condition to the Waiting to go up or down variables; 
3.All other locations which were required implemented with casual semaphores;

To create, Run:
G++ -o main.exe main.c