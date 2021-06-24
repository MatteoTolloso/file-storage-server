#include <mylinkedlist.h>

void init(struct node **head, int data)
{
	EXIT_ON(*head = (struct node *)malloc(sizeof(struct node)), == NULL);

	(*head)->data = data;
	(*head)->next = NULL;

	return;
}

void list_insert(struct node **head, int data)
{

    if (*head == NULL){
        init(head,data);
		return;
    }

	struct node *current = *head;
	struct node *tmp;

	while(current){
		if(current->data == data) return;
		tmp = current;
		current = current->next;
	} 

	/* create a new node after tmp */
	struct node *new_node;
    EXIT_ON(new_node = (struct node *)malloc(sizeof(struct node)), == NULL);
	
	new_node->next = NULL;
	new_node->data = data;

	tmp->next = new_node;

	return;

}

int list_mem(struct node **head, int data)
{

    if (*head == NULL){
        return 0;
    }

	struct node *current = *head;

	while (current){

        if (current->data == data){
            return 1;
        }

		current = current->next;
	} 

	return 0;

}


void list_remove(struct node **head, int data)
{
	struct node *current = *head;
	struct node *prev = NULL;

	do {
		if (current->data == data) {
			break;
		}
		prev = current;
		current = current->next;
	} while (current);

	/* if the first element */
	if (current == *head) {
		/* reuse prev */
		prev = *head;
		*head = current->next;
		free(prev);
		return;
	}

	/* if the last element */
	if (current->next == NULL) {
		prev->next = NULL;
		free(current);
		return;
	}

	prev->next = current->next;
	free(current);
	return;
}


void list_deinit(struct node **head)
{
	struct node *node = *head;
	do {
		struct node *tmp;
		tmp = node;
		node = node->next;
		free(tmp);
	} while (node);
}