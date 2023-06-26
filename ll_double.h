/*
 * Copyright (c) 2017, Hammurabi Mendes.
 * License: BSD 2-clause
 */
#ifndef LL_DOUBLE_H
#define LL_DOUBLE_H

struct node {
	void *data;

	struct node *prev;
	struct node *next;
};

struct list {
	struct node *head;
	struct node *tail;
};

void ll_init(struct list *list);
struct node *ll_insert_head(struct list *list, void *data);
struct node *ll_insert_tail(struct list *list, void *data);
struct node *ll_search(struct list *list, void *data);
struct node *ll_search_comparison(struct list *list, void *data, int (*compare)(void *, void *));
struct node *ll_search_predicate(struct list *list, int (*predicate)(void *));
struct node *ll_remove(struct list *list, void *data);
struct node *ll_remove_head(struct list *list);
struct node *ll_remove_tail(struct list *list);

#endif /* LL_DOUBLE_H */
