#include "synchronization.h"

#include <stdlib.h>

#include "threads.h"

struct waiter_t {
    int thread_id;
};

int thread_mutex_init(thread_mutex_t *mutex) {
    if(mutex) {
        atomic_init(&(mutex->locked), 0);
    }

    return 0;
}

int thread_mutex_lock(thread_mutex_t *mutex) {
    while(1) {
        // An empty while loop. In production code, you would insert
        // a "pause" assembly instruction inside the loop (on Intel)
        // in order to clear speculative operations in the CPU pipeline.
        while(mutex->locked)
            ; // spin!

        int expected = 0;

        if(atomic_compare_exchange_strong(&(mutex->locked), &expected, 1)) {
            break;
        }
    }

    return 0;
}

int thread_mutex_unlock(thread_mutex_t *mutex) {
    atomic_store(&mutex->locked, 0);

    return 0;
}

int thread_cond_init(thread_cond_t *condition_variable) {
    if(thread_mutex_init(&condition_variable->internal_mutex)) {
        ll_init(&condition_variable->waiters_list);
        return 0;
    }
    return 1;
}
/*
* function adds the current thread number to the list of waiters,
* and changes the state of the current thread to Blocked. The
* manipulation of the list of waiters associated with the condition
* variable passed as parameter should be done only after you acquire
* the internal mutex of that condition variable, otherwise multiple
* threads that call thread_cond_wait() could operate at the same time
* in that list and corrupt it. After you safely added the current 
* thread number to the list of waiters, call thread_yield() to yield
* control to another thread.
*/
int thread_cond_wait(thread_cond_t *condition_variable, thread_mutex_t *mutex) {
    thread_mutex_lock(&condition_variable->internal_mutex);

    // insert data
    ll_insert_tail(&condition_variable->waiters_list, &current_thread_context->number);

    // unlock threads
    thread_mutex_unlock(mutex);
    thread_mutex_unlock(&condition_variable->internal_mutex);

    // block current thread
    current_thread_context->state = STATE_BLOCKED;

    thread_yield();
    thread_mutex_lock(mutex);
    return 0;
}

/*
* function removes the first waiter of the condition variable’s waiter’s
* list if there is one. After doing that, you set the corresponding
* thread’s state to Active. All of those operations should be done 
* holding the internal mutex of the condition variable, because no two
* threads can be operating on a condition variable object at the same time.
*/
int thread_cond_signal(thread_cond_t *condition_variable) {
    thread_mutex_lock(&condition_variable->internal_mutex);

    struct node *head_t = ll_remove_head(&condition_variable->waiters_list);

    if(head_t == NULL) {
        thread_mutex_unlock(&condition_variable->internal_mutex);
        return 1;
    }
    // get int from data
    int index_t = *((int *) (head_t->data));
    thread_context[index_t].state = STATE_ACTIVE;
    thread_mutex_unlock(&condition_variable->internal_mutex);
    return 0;
}

/* 
* Broadcast to all threads in waiter list.
*/
int thread_cond_broadcast(thread_cond_t *condition_variable) {
    while(condition_variable->waiters_list.head != NULL) {
        thread_cond_signal(condition_variable);
    }
    return 0;
}
