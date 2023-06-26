#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

#include "threads.h"
#include "synchronization.h"

#define NUM_THREADS 8

int queue_size = 0;

thread_mutex_t queue_lock;

void *producer(void * data) {
	while(1) {
		thread_mutex_lock(&queue_lock);
		queue_size++;
		printf("Produced: virtual queue has size %d\n", queue_size);
		fflush(stdout);
		thread_mutex_unlock(&queue_lock);

		// *Exchange* the comment on those lines when your preemption is working
		//usleep(100);
		thread_yield();
	}

	thread_exit(NULL);
	return NULL;
}

void *consumer(void * data) {
	while(1) {
		thread_mutex_lock(&queue_lock);

		if(queue_size == 0) {
			printf("DID NOT CONSUME: virtual queue has size %d\n", queue_size);
			fflush(stdout);
		}
		else {
			queue_size--;
			printf("Consumed: virtual queue has size %d\n", queue_size);
			fflush(stdout);
		}

		thread_mutex_unlock(&queue_lock);

		// *Exchange* the comment on those lines when your preemption is working
		//usleep(100);
		thread_yield();
	}

	thread_exit(NULL);
	return NULL;
}

int main(void) {
	int producerID;
	int consumerID;

	// Initialize the threading package
	thread_init(0);

	// Initialize the mutex
	thread_mutex_init(&queue_lock);

	// Initialize threads
	producerID = thread_create(producer, NULL);
	consumerID = thread_create(consumer, NULL);

	// Join threads
	thread_join(consumerID);
	thread_join(producerID);

	return EXIT_SUCCESS;
}
