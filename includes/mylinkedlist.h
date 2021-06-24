#ifndef _MYLINKEDLIST_H
#define _MYLINKEDLIST_H
#include <stdio.h>
#include <stdlib.h>
#include <myutil.h>

typedef struct node {
	int data;
	struct node *next;
}List_t;

void init(struct node **head, int data);

void list_insert(struct node **head, int data);

void list_remove(struct node **head, int data);

void list_deinit(struct node **head);

int list_mem(struct node **head, int data);
#endif /* _MYLINKEDLIST_H */