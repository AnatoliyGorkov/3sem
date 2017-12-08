#ifndef _LIST_USAGE_

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define LIST_TYPE int

typedef struct _ListElement
{
	LIST_TYPE value;
	struct _ListElement* prev;
	struct _ListElement* next;
} ListElement;

typedef struct _List
{
	size_t size;
	ListElement* head;
} List;


List* listConstruct();
int listDestruct(List* list);
ListElement* getElement(const List* list, size_t position);
int listInsert(List* list, const LIST_TYPE data, size_t position);
int listRemoveByValue(List* list, const LIST_TYPE value);
size_t listFind(const List* list, const LIST_TYPE value);
int listForeach(List* list, int (*func) (LIST_TYPE*));
LIST_TYPE* listToArray(const List* list);


List* listConstruct()
{
	List* list = (List*) calloc(1, sizeof(*list));
	return list;
}

int listDestruct(List* list)
{
	if (list == NULL)
		return 1;
	if (list -> size == 0)
	{
		free(list);
		return 0;
	}
	if (list -> size == 1)
	{
		free(list -> head);
		free(list);
		return 0;
	}
	ListElement* currentElement = list -> head -> next;
	ListElement* previousElement = list -> head;
	do
	{
		free(previousElement);
		previousElement = currentElement;
		currentElement = currentElement -> next;

	} while (currentElement != list -> head);
	free(previousElement);
	free(list);
	return 0;
}

ListElement* getElement(const List* list, size_t position) //numeration starts with 1
{
	if (list == NULL || position > list -> size)
		return NULL;
	size_t i;
	ListElement* element = list -> head;
	if (position <= list -> size / 2)
	{
		for(i = 1; i < position; i++)
			element = element -> next;
	}
	else
	{
		for(i = list -> size; i >= position; i--)
			element = element -> prev;
	}
	return element;
}

int listInsert(List* list, const LIST_TYPE data, size_t position)
{
	if (list == NULL || position > list -> size + 1)
		return 1;
	ListElement* element = (ListElement*) malloc(sizeof(*element));
	if (element == NULL)
		return 1;
	element -> value = data;
	if (list -> size == 0)
	{
		if (position != 1)
		{
			free(element);
			return 1;
		}
		element -> next = element;
		element -> prev = element;
		list -> head = element;
		list -> size++;
		return 0;
	}
	ListElement* currentElement;
	if (position == list -> size + 1)
		currentElement = list -> head;
	else
		currentElement = getElement(list, position);
	(currentElement -> prev) -> next = element;
	element -> prev = currentElement -> prev;
	currentElement -> prev = element;
	element -> next = currentElement;
	if (position == 1)
		list -> head = element;
	list -> size++;
	return 0;
}

int listRemoveByValue(List* list, const LIST_TYPE value) //compar return 0 when equal
{
	if (list == NULL || list -> head == NULL || list -> size == 0)
		return 1;
	ListElement* element = list -> head;
	size_t count = 0;
	char found = 0;

	do
	{
		count++;
		if (element -> value == value)
		{
			found = 1;
			break;
		}
		element = element -> next;
	} while(element != list -> head);

	if(!found)
		return 1;
	if (count == 1)
	{
		if (list -> size == 1)
		{
			list -> head = NULL;
			free(element);
			list -> size--;
			return 0;
		}
		else
			list -> head = element -> next;
	}
	(element -> prev) -> next = element -> next;
	(element -> next) -> prev = element -> prev;
	free(element);
	list -> size--;
	return 0;
}


size_t listFind(const List* list, const LIST_TYPE value)
{
	if (list == NULL || list -> head == NULL || list -> size == 0)
		return 0;
	ListElement* element = list -> head;
	size_t count = 0;
	do
	{
		count++;
		if (element -> value ==  value)
			return count;
		element = element -> next;
	} while(element != list -> head);
	return 0; 
}

LIST_TYPE* listToArray(const List* list) //has to be freed after usage!!!
{
	if (list == NULL || list -> size == 0)
		return NULL;
	LIST_TYPE* array = (LIST_TYPE*) malloc(sizeof(*array) * list -> size);
	if (array == NULL)
		return NULL;
	size_t i = 0;
	ListElement* element = list -> head;
	do
	{
		*(array + i) = element -> value;
		i++;
		element = element -> next;
	} while(element != list -> head);
	return array;
}

#define _LIST_USAGE_ 1
#endif
