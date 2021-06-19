#ifndef _MYLINKEDLIST_H
#define _MYLINKEDLIST_H
#include <stdio.h>
#include <stdlib.h>
#include <myutil.h>

struct node {
	int data;
	struct node *next;
};

int init(struct node **head, int data);

int list_insert(struct node **head, int data);

void list_remove(struct node **head, int data);


void list_deinit(struct node **head);

#endif /* _MYLINKEDLIST_H */