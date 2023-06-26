#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

#include "threads.h"
#include "synchronization.h"

#define NUM_THREADS 8

int queue_size = 0;

thread_mutex_t queue_lock;
thread_cond_t queue_not_empty;

void *producer(void * data) {
	while(1) {
		thread_mutex_lock(&queue_lock);
		queue_size++;
		printf("Produced: virtual queue has size %d\n", queue_size);
		fflush(stdout);
		thread_cond_signal(&queue_not_empty);
		thread_mutex_unlock(&queue_lock);

		thread_yield();
		// usleep(100);
	}

	thread_exit(NULL);
	return NULL;
}

void *consumer(void * data) {
	while(1) {
		thread_mutex_lock(&queue_lock);

		while(queue_size == 0) {
			printf("DID NOT CONSUME: virtual queue has size %d\n", queue_size);
			fflush(stdout);
			thread_cond_wait(&queue_not_empty, &queue_lock);
		}

		queue_size--;
		printf("Consumed: virtual queue has size %d\n", queue_size);
		fflush(stdout);
		thread_mutex_unlock(&queue_lock);
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

	// Initialize the condition variable
	thread_cond_init(&queue_not_empty);

	// Initialize threads
	producerID = thread_create(producer, NULL);
	consumerID = thread_create(consumer, NULL);

	// Join threads
	thread_join(consumerID);
	thread_join(producerID);

	return EXIT_SUCCESS;
}
