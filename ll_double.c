/*
 * Copyright (c) 2017, Hammurabi Mendes.
 * License: BSD 2-clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ll_double.h"

void ll_init(struct list *list) {
	if(!list) {
		return;
	}

	list->head = NULL;
	list->tail = NULL;
}

struct node *ll_insert_head(struct list *list, void *data) {
	struct node *new_node;

	new_node = (struct node *) malloc(sizeof(struct node));

	if(new_node != NULL) {
		new_node->data = data;

		new_node->prev = NULL;
		new_node->next = list->head;

		if(list->head != NULL) {
			list->head->prev = new_node;
		}

		list->head = new_node;

		if(list->tail == NULL) {
			list->tail = new_node;
		}
	}

	return new_node;
}

struct node *ll_insert_tail(struct list *list, void *data) {
	struct node *new_node;

	new_node = (struct node *) malloc(sizeof(struct node));

	if(new_node != NULL) {
		new_node->data = data;

		new_node->prev = list->tail;
		new_node->next = NULL;

		if(list->tail != NULL) {
			list->tail->next = new_node;
		}

		list->tail = new_node;

		if(list->head == NULL) {
			list->head = new_node;
		}
	}

	return new_node;
}

struct node *ll_search(struct list *list, void *data) {
	struct node *temporary;

	temporary = list->head;

	while(temporary != NULL) {
		if(data == temporary->data) {
			return temporary;
		}

		temporary = temporary->next;
	}

	return NULL;
}

struct node *ll_search_comparison(struct list *list, void *data, int (*compare)(void *, void *)) {
	struct node *temporary;

	temporary = list->head;

	while(temporary != NULL) {
		if(compare(data, temporary->data) == 1) {
			return temporary;
		}

		temporary = temporary->next;
	}

	return NULL;
}

struct node *ll_search_predicate(struct list *list, int (*predicate)(void *)) {
	struct node *temporary;

	temporary = list->head;

	while(temporary != NULL) {
		if(predicate(temporary->data) == 1) {
			return temporary;
		}

		temporary = temporary->next;
	}

	return NULL;
}

struct node *ll_remove(struct list *list, void *data) {
	struct node *temporary = ll_search(list, data);

	if(temporary == NULL) {
		return NULL;
	}

	if(temporary->prev != NULL) {
		temporary->prev->next = temporary->next;
	}
	else {
		list->head = temporary->next;
	}

	if(temporary->next != NULL) {
		temporary->next->prev = temporary->prev;
	}
	else {
		list->tail = temporary->prev;
	}

	return data;
}

struct node *ll_remove_head(struct list *list) {
	if(!list->head) {
		return NULL;
	}

	struct node *result = list->head;

	list->head = list->head->next;

	if(list->head) {
		list->head->prev = NULL;
	}
	else {
		list->tail = NULL;
	}

	return result;
}

struct node *ll_remove_tail(struct list *list) {
	if(!list->tail) {
		return NULL;
	}

	struct node *result = list->tail;

	list->tail = list->tail->prev;

	if(list->tail) {
		list->tail->next = NULL;
	}
	else {
		list->head = NULL;
	}

	return result;
}
