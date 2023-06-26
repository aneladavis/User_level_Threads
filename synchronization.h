/*
 * Copyright (c) 2017, Hammurabi Mendes.
 * License: BSD 2-clause
 */
#ifndef SYNCHRONIZATION_H
#define SYNCHRONIZATION_H

#include <stdatomic.h>

#include "ll_double.h"

typedef struct thread_t_ thread_t;

struct thread_mutex_t_ {
	atomic_int locked;
};

typedef struct thread_mutex_t_ thread_mutex_t;

struct thread_cond_t_ {
	thread_mutex_t internal_mutex;

	struct list waiters_list;
};

typedef struct thread_cond_t_ thread_cond_t;

int thread_mutex_init(thread_mutex_t *mutex);
int thread_mutex_lock(thread_mutex_t *mutex);
int thread_mutex_unlock(thread_mutex_t *mutex);

int thread_cond_init(thread_cond_t *condition_variable);
int thread_cond_wait(thread_cond_t *condition_variable, thread_mutex_t *mutex);
int thread_cond_signal(thread_cond_t *condition_variable);
int thread_cond_broadcast(thread_cond_t *condition_variable);

#endif /* SYNCHRONIZATION_H */
