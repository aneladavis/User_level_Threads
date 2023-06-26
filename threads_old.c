/*
 * 2023, Anela Davis
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <setjmp.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>


#include "threads.h"
#include "synchronization.h"


char stacks[MAX_THREADS][STACK_SIZE];

struct itimerval alarm_set;

int new_thread;
int created = 0;
int preemption = 0;

tcb_t thread_context[MAX_THREADS];
tcb_t *current_thread_context;


#define MAX_THREADS 128

void sigusr_handler();
void setup_signal_handler(int signal, void (*handler)(int));
void alarm_handler();



void sigusr_handler() { // look into more
    if(setjmp(thread_context[new_thread].buffer) == 0) {
        created = 1;
    }
    else {
        current_thread_context->function(current_thread_context->argument);
    }
}

void set_time(int usec){
    // Setup periodic alarm
	struct itimerval timer;

	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = usec;

	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = usec;

	if(setitimer(ITIMER_REAL, &timer, NULL) == -1) {
		perror("setitimer");
		exit(EXIT_FAILURE);
	}
}

void disable(int usec){
    // Setup periodic alarm
	struct itimerval timer;

	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = usec * 200;

	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = usec * 200;

	if(setitimer(ITIMER_REAL, &timer, NULL) == -1) {
		perror("setitimer");
		exit(EXIT_FAILURE);
	}
}
void setup_signal_handler(int signal, void (*handler)(int)) {
    struct sigaction options;

    memset(&options, 0, sizeof(struct sigaction));

    options.sa_handler = handler;

    if(sigaction(signal, &options, NULL) == -1) {
        perror("sigaction");

        exit(EXIT_FAILURE);
    }
    set_time(100000);
}

// should initialize the thread_context array (making all entries Invalid),
// and then create a first entry for the main program. For that particular entry, make the
// function, argument, and return value be NULL. It’s assumed that the function of the
// main thread is the main() function. The state of this thread should be STATE_ACTIVE.
// Also, thread_init() should setup the signal handlers necessary for thread creation,
// following the thread creation variant that we discussed in class (see newthread.c on Moodle).
void thread_init(int preemption_enabled) {
    if (preemption_enabled == 1){
        preemption = 1;
        setup_signal_handler(SIGALRM, alarm_handler);
    }

    else{
        preemption = 0;
    }

    for(int i = 1; i < MAX_THREADS; i++) {
        thread_context[i].state = STATE_INVALID;
    }
    thread_context[0].number = 0;
    thread_context[0].state = STATE_ACTIVE;
    thread_context[0].function = NULL;
    thread_context[0].argument = NULL;
    thread_context[0].return_value = NULL;
    thread_context[0].joiner_thread_number = -1;
    thread_context[0].stack = &stacks[0][0]; 
    current_thread_context = &thread_context[0];

    // Setup signal handler

    struct sigaction sigusr_hints;

    memset(&sigusr_hints, 0, sizeof(struct sigaction));
    sigusr_hints.sa_handler = sigusr_handler;
    sigusr_hints.sa_flags = SA_ONSTACK; // <<-- Look at this
    sigemptyset(&sigusr_hints.sa_mask);

    if(sigaction(SIGUSR1, &sigusr_hints, NULL) == -1) {
        perror("sigaction/SIGUSR1");
        exit(EXIT_FAILURE);
    }
}

/*
 * Whenever the main thread wants to create a new thread, it will call thread_create(). 
 * Please see an example in producer_consumer1.c. This function goes over the thread_context
 * array, finds an empty entry and initializes it with the function and arguments provided as 
 * parameters. The new thread’s state is set to Active. Each of these newly created threads 
 * has a separate stack. You should declare an array containing multiple chunks of memory of 
 * size 64K, each chunk being the stack for each newly created thread. After initializing 
 * the state, you should use the thread creation method that we used in class and effectively 
 * create the thread. The function returns the ID of the new thread.
 */
int thread_create(void *(*function)(void *), void *argument) {
    int i = 0;
    int foundFlag = 0;
    while(thread_context[i].state != STATE_INVALID && i < MAX_THREADS) {
        i++;
        foundFlag = 1;
    }
    if(foundFlag == 1) {
        new_thread = i;
        thread_context[new_thread].state = STATE_ACTIVE;
        thread_context[new_thread].argument = argument;
        thread_context[new_thread].function = function;
        thread_context[new_thread].return_value = NULL;
        thread_context[new_thread].stack = &stacks[new_thread][0];
        thread_context[new_thread].joiner_thread_number = -1;
        thread_context[new_thread].number = i;

        // this code borrowed from Dr. Mendes' newthread.c
        // ask about that 

        stack_t new_stack;
        stack_t old_stack;

        new_stack.ss_flags = 0;
        new_stack.ss_size = STACK_SIZE;
        new_stack.ss_sp = &stacks[new_thread][0];

        if(sigaltstack(&new_stack, &old_stack) == -1) {
            perror("sigaltstack");
            exit(EXIT_FAILURE);
        }
        raise(SIGUSR1);

        while(!created) {}; // spin!
        created = 0;


        fflush(stdout);

        return i;
    }
    perror("Thread Creation error!");
    return EXIT_FAILURE;
}

/*
* This function is your scheduler. When a running thread calls thread_yield(),
* you should look for the next available thread for running (i.e., a thread in the Active state), 
* and switch context to it using setjmp()/longjmp(). If no other thread besides the current running
* one is active, you should return 0. Make sure to change the current_thread_context variable to the
* TCB of the new thread when you change context.
*/
int thread_yield() {
    // disable preemption
    // if (preemption == 1){
    //     disable(10000);
    // }
    for(int i = (1 + current_thread_context->number) % MAX_THREADS; i != current_thread_context->number; i ++){
        // printf("%d\n", i);
        if(i >= MAX_THREADS) { // HM
            i = 0;
        }
        if(thread_context[i].state == STATE_ACTIVE){
            if(setjmp(current_thread_context->buffer) == 0){
                current_thread_context = &thread_context[i];
                // if (preemption == 1){
                //     set_time(10000);
                // }
                longjmp(current_thread_context->buffer, 1);
            }
            break;
        }
    }
    return 1;
}

/*
* When a running thread calls thread_exit(), its state is marked as Finished, and the return_value
* field of the TCB entry for the current thread is set to the provided returned value.
* If another thread has been waiting for this exiting thread’s termination, its variable
* joiner_thread_number will be set with the ID of the thread that is waiting. In that case,
* immediately switch context to that (currently blocked) waiting thread, which should be 
* reactivated upon restart (change its status back to Active). If the variable joiner_thread_number
* indicates that there is no joiner, call your scheduler in thread_yield() to find another thread to run.
*/
void thread_exit(void *return_value) {
    // disable preemption
    // if (preemption == 1){
    //     disable(10000);
    // }

    current_thread_context->state = STATE_FINISHED;
    current_thread_context->return_value = return_value;

    if(current_thread_context->joiner_thread_number >= 0 && setjmp(current_thread_context->buffer) == 0){
        thread_context[current_thread_context->joiner_thread_number].state = STATE_ACTIVE;
        current_thread_context = &thread_context[current_thread_context->joiner_thread_number];
        //enable preemption
        // if (preemption == 1){
        //     set_time(10000);
        // }
        longjmp(current_thread_context->buffer, 1);
    }
    else{
        //enable preemption
        // if (preemption == 1){
        //     set_time(10000);
        // }
        thread_yield();
    }
    //enable preemption
    // if (preemption == 1){
    //     set_time(10000);
    // }
}

void thread_join(int target_thread_number) {
    // disable preemption
    // if (preemption == 1){
    //     disable(10000);
    // }

    if(thread_context[target_thread_number].state == STATE_FINISHED){
        thread_context[target_thread_number].state = STATE_INVALID;
    }
    else{
        current_thread_context->state = STATE_BLOCKED; // implement preemption
        thread_context[target_thread_number].joiner_thread_number = current_thread_context->number;

        if(setjmp(current_thread_context->buffer) == 0){
            thread_context[target_thread_number].state = STATE_ACTIVE;
            current_thread_context = &thread_context[target_thread_number];
            //enable preemption
            // if (preemption == 1){
            //     set_time(10000);
            // }
            longjmp(current_thread_context->buffer, 1);
        }
        else{
            thread_context[target_thread_number].state = STATE_INVALID;
        }
    }
    //enable preemption
    // if (preemption == 1){
    //     set_time(10000);
    // }
}

void alarm_handler() {
    thread_yield();
}
