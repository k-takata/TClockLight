/*-------------------------------------------------------------
  list.c : linked list
  (C) 1997-2004 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon
---------------------------------------------------------------*/

#include "common.h"

void *add_listitem(void *top, void *item)
{
	struct list *current, *last = NULL;
	
	current = (struct list *)top;
	while(current)
	{
		last = current;
		current = current->next;
	}
	
	((struct list *)item)->next = NULL;
	
	if(last) last->next = item;
	else top = item;
	
	/*{
		char s[80];
		wsprintf(s, "%s is added", ((struct list *)item)->name);
		WriteDebug(s);
	}*/
	
	return top;
}

void *copy_listitem(void *top, void *item, size_t size)
{
	void *newitem = malloc(size);
	memcpy(newitem, item, size);
	return add_listitem(top, newitem);
}

void *del_listitem(void *top, void *item)
{
	struct list *current, *before;
	
	current = top;
	before = NULL;
	while(current)
	{
		if(current == item)
		{
			if(before) before->next = current->next;
			else top = current->next;
			
			/*{
				char s[80];
				wsprintf(s, "%s is free", current->name);
				WriteDebug(s);
			}*/
			
			free(current);
			break;
		}
		before = current;
		current = current->next;
	}
	
	return top;
}

void *clear_list(void *top)
{
	struct list *current = top;
	while(current)
	{
		struct list *temp = current;
		current = current->next;
		
		/*{
			char s[80];
			wsprintf(s, "%s is free", temp->name);
			WriteDebug(s);
		}*/
		
		free(temp);
	}
	return NULL;
}

void *get_listitem(void *top, int index)
{
	int i;
	struct list *current = top;
	
	if(index < 0) return NULL;
	for(i = 0; i < index && current; i++)
		current = current->next;
	
	return current;
}

