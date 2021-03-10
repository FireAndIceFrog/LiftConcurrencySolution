// --------------------------------------------------
// ---   159.341 Assignment 2 - Lift Simulator    ---
// --------------------------------------------------
// Assignment Fixed by Mathew Lawrence 17354272
// Written by M. J. Johnson
// Updated by D. P. Playne - 2019
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <windows.h>
#include "lift.h"

// --------------------------------------------------
// Define Problem Size
// --------------------------------------------------
#define NLIFTS 4          // The number of lifts in the building
#define NFLOORS 20        // The number of floors in the building
#define NPEOPLE 20        // The number of people in the building
#define MAXNOINLIFT 10    // Maximum number of people in a lift
#define ENTER 1
#define EXIT 0

// --------------------------------------------------
// Define delay times (in milliseconds)
// --------------------------------------------------
#define FAST
// #define FAST

#if defined(SLOW)
	#define LIFTSPEED 50      // The time it takes for the lift to move one floor
	#define GETINSPEED 50     // The time it takes to get into the lift
	#define GETOUTSPEED 50    // The time it takes to get out of the lift
	#define PEOPLESPEED 100   // The maximum time a person spends on a floor
#elif defined(FAST)
	#define LIFTSPEED 0       // The time it takes for the lift to move one floor
	#define GETINSPEED 0      // The time it takes to get into the lift
	#define GETOUTSPEED 0     // The time it takes to get out of the lift
	#define PEOPLESPEED 1     // The maximum time a person spends on a floor
#endif

// --------------------------------------------------
// Define lift directions (UP/DOWN)
// --------------------------------------------------
#define UP 1
#define DOWN -1

// --------------------------------------------------
// Information about a lift
// --------------------------------------------------
typedef struct {
	long long no;                       // The lift number (id)
	int position;                 // The floor it is on
	int direction;                // Which direction it is going (UP/DOWN)
	int peopleinlift;             // The number of people in the lift
	int stops[NFLOORS];           // How many people are going to each floor
	semaphore stopsem[NFLOORS];   // People in the lift wait on one of these
} lift_info;
// --------------------------------------------------
// Information about a floor in the building
// --------------------------------------------------
typedef struct {
	int waitingtogoup;      // The number of people waiting to go up
	int waitingtogodown;    // The number of people waiting to go down
	semaphore up_arrow;     // People going up wait on this
	semaphore down_arrow;   // People going down wait on this
	lift_info *currentLift;	//current lift at the floor
} floor_info;
// --------------------------------------------------
// Some global variables
// --------------------------------------------------
floor_info floors[NFLOORS];
semaphore floorLock[NFLOORS];
semaphore printLock;


// --------------------------------------------------
// Print a string on the screen at position (x,y)
// --------------------------------------------------
void print_at_xy(int x, int y, const char *s) {
	semaphore_wait(&(printLock));
	// Move cursor to (x,y)
	gotoxy(x,y);
	
	// Slow things down
	Sleep(1);

	// Print the string
	printf("%s", s);
	
	// Move cursor out of the way
	gotoxy(42, NFLOORS+2);
	ReleaseMutex(printLock);
}

// --------------------------------------------------
// Function for a lift to pick people waiting on a
// floor going in a certain direction
// --------------------------------------------------
void get_into_lift(lift_info *lift, int direction) {
	// Local variables
	int *waiting;
	semaphore *s;
	semaphore_wait(&(floorLock[lift->position]));
	// Check lift direction
	if(direction==UP) {
		// Use up_arrow semaphore
		s = &floors[lift->position].up_arrow;

		// Number of people waiting to go up
		waiting = &floors[lift->position].waitingtogoup;
	} else {
		// Use down_arrow semaphore
		s = &floors[lift->position].down_arrow;

		// Number of people waiting to go down
		waiting = &floors[lift->position].waitingtogodown;
	}
	
	//set the floor to current floor.
	floors[lift->position].currentLift = lift;

	
	// For all the people waiting
	while(*waiting) {
		
		// Check if lift is empty
		if(lift->peopleinlift == 0) {
			// Set the direction of the lift
			lift->direction = direction;
		}
		
		// Check there are people waiting and lift isn't full
		if(lift->peopleinlift < MAXNOINLIFT && *waiting) {

			// Add person to the lift
			lift->peopleinlift++;
			// Erase the person from the waiting queue
			print_at_xy(NLIFTS*4+floors[lift->position].waitingtogodown + floors[lift->position].waitingtogoup, NFLOORS-lift->position, " ");
			// One less person waiting
			(*waiting)--;
			// Wait for person to get into lift
			Sleep(GETINSPEED);
				
			
			semaphore_signal(s);
			// Signal passenger to enter
			
			
			
			
		
		 } else {
			break;
		}
	}
	ReleaseMutex(floorLock[lift->position]);
	
	
	
}

// --------------------------------------------------
// Function for the Lift Threads
// --------------------------------------------------
void* lift_thread(void *p) {
	// Local variables
	lift_info lift;
	int no = (long long)p;
	int i;
	
	// Set up Lift
	lift.no = no;           // Set lift number
	lift.position = 0;      // Lift starts on ground floor
	lift.direction = UP;    // Lift starts going up
	lift.peopleinlift = 0;  // Lift starts empty

	for(i = 0; i < NFLOORS; i++) {
		lift.stops[i]=0;                        // No passengers waiting
		semaphore_create(&lift.stopsem[i], 0);  // Initialise semaphores
	}
 
	// Randomise lift
	randomise();

	// Wait for random start up time (up to a second)
	Sleep(rnd(1000));

	// Loop forever
	while(1) {
		// Print current position of the lift
		print_at_xy(no*4+1, NFLOORS-lift.position, lf);

		// Wait for a while
		Sleep(LIFTSPEED);

		// Drop off passengers on this floor
		while (lift.stops[lift.position] != 0) {
			// One less passenger in lift
			lift.peopleinlift--;

			// One less waiting to get off at this floor
			lift.stops[lift.position]--;
			
			// Wait for exit lift delay
			Sleep(GETOUTSPEED);
			semaphore_signal(&(lift.stopsem[lift.position]));
		// Signal passenger to leave lift			

			// Check if that was the last passenger waiting for this floor
			if(!lift.stops[lift.position]) {
				// Clear the "-"
				print_at_xy(no*4+1+2, NFLOORS-lift.position, " ");
			}
		}
		// Check if lift is going up or is empty
		if(lift.direction==UP || !lift.peopleinlift) {
			// Pick up passengers waiting to go up
			get_into_lift(&lift, UP);
		}
		// Check if lift is going down or is empty
		if(lift.direction==DOWN || !lift.peopleinlift) {
			// Pick up passengers waiting to go down
			get_into_lift(&lift, DOWN);
		}
		
		// Erase lift from screen
		print_at_xy(no*4+1, NFLOORS-lift.position, (lift.direction + 1 ? " " : lc));
		
		// Move lift
		lift.position += lift.direction;

		// Check if lift is at top or bottom
		if(lift.position == 0 || lift.position == NFLOORS-1) {
			// Change lift direction
			lift.direction = -lift.direction;
		}
		
	}
	
	return NULL;
}

// --------------------------------------------------
// Function for the Person Threads
// --------------------------------------------------
void* person_thread(void *p) {
	// Local variables
	int from = 0, to; // Start on the ground floor
	lift_info *lift;

	// Randomise
	randomise();

	// Stay in the building forever
	while(1) {
		semaphore *s; 
		// Work for a while
		Sleep(rnd(PEOPLESPEED));
		do {
			// Randomly pick another floor to go to
			to = rnd(NFLOORS);
		} while(to == from);
		semaphore_wait(&(floorLock[from]));
		// Check which direction the person is going (UP/DOWN)
		if(to > from) {
			// One more person waiting to go up
			floors[from].waitingtogoup++;
			
			// Print person waiting
			print_at_xy(NLIFTS*4+ floors[from].waitingtogoup +floors[from].waitingtogodown,NFLOORS-from, pr);
			// Wait for a lift to arrive (going up)
			s = &(floors[from].up_arrow);
			
		} else {
			// One more person waiting to go down
			floors[from].waitingtogodown++;
			
			// Print person waiting
			print_at_xy(NLIFTS*4+floors[from].waitingtogodown+floors[from].waitingtogoup,NFLOORS-from, pr);
			s = &(floors[from].down_arrow);
			// Wait for a lift to arrive (going down)
			
		}
		ReleaseMutex(floorLock[from]);
		semaphore_wait(s);
		
		// Which lift we are getting into
		lift = floors[from].currentLift;
		// semaphore_wait(&(liftLock[(int)lift->no]));
		// Add one to passengers waiting for floor
		lift->stops[to]++;

		// Press button if we are the first
		if(lift->stops[to]==1) {
			// Print light for destination
			print_at_xy(lift->no*4+1+2, NFLOORS-to, "-");
		}
		// ReleaseMutex(liftLock[(int)lift->no]);
		// Wait until we are at the right floor
		semaphore_wait(&(lift->stopsem[to]));
		
		// Exit the lift
		from = to;
	}
	
	return NULL;
}

// --------------------------------------------------
//	Print the building on the screen
// --------------------------------------------------
void printbuilding(void) {
	// Local variables
	int l,f;

	// Clear Screen
	system(clear_screen);
	
	// Print Roof
	printf("%s", tl);
	for(l = 0; l < NLIFTS-1; l++) {
		printf("%s%s%s%s", hl, td, hl, td);
	}
	printf("%s%s%s%s\n", hl, td, hl, tr);

	// Print Floors and Lifts
	for(f = NFLOORS-1; f >= 0; f--) {
		for(l = 0; l < NLIFTS; l++) {
			printf("%s%s%s ", vl, lc, vl);
			if(l == NLIFTS-1) {
				printf("%s\n", vl);
			}
		}
	}

	// Print Ground
	printf("%s", bl);
	for(l = 0; l < NLIFTS-1; l++) {
		printf("%s%s%s%s", hl, tu, hl, tu);
	}
	printf("%s%s%s%s\n", hl, tu, hl, br);

	// Print Message
	printf("-------------------------------------\n");
	printf("159.341 Assignment 2 semester 1 2020\n");
	printf("Submitted by: Mathew Lawrence 17354272\n");
	printf("-------------------------------------\n");
	printf("Lift Simulation - Press CTRL-Break to exit\n");
}

// --------------------------------------------------
// Main starts the threads and then waits.
// --------------------------------------------------
int main() {
	// Local variables
	unsigned long i;
	printLock = CreateMutex( 
        NULL,              // default security attributes
        FALSE,             // initially not owned
        NULL);             // unnamed mutex
	for (i = 0; i < NFLOORS; ++i){
		floorLock[i] = CreateMutex( 
        NULL,              // default security attributes
        FALSE,             // initially not owned
        NULL);             // unnamed mutex
	}
	// Initialise Building
	for(i = 0; i < NFLOORS; i++) {
		// Initialise Floor
		floors[i].waitingtogoup = 0;
		floors[i].waitingtogodown = 0;
		semaphore_create(&floors[i].up_arrow, 0);
		semaphore_create(&floors[i].down_arrow, 0);
	}

	// --- Initialise any other semaphores ---
	
	// Print Building
	printbuilding();

	// Create Lifts
	for(i = 0; i < NLIFTS; i++) {
		// Create Lift Thread
		create_thread(lift_thread, (void*)i);
	}

	// Create People
	for(i = 0; i < NPEOPLE; i++) {
		// Create Person Thread
		create_thread(person_thread, (void*)i);
	}

	// Go to sleep for 86400 seconds (one day)
	Sleep(86400000ULL);
}


