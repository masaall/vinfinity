
#include "types.h"
#include "defs.h"
#include "list.h"

void list_init(struct list *list){
	list->base.prev = &list->base;
	list->base.next = &list->base;
}

void list_insert(struct list *list, struct list_elem *elem){
	elem->next = list->base.next;
	elem->prev = &list->base;
	list->base.next->prev = elem;
	list->base.next = elem;
}

void list_remove(struct list_elem *elem){
	elem->next->prev = elem->prev;
	elem->prev->next = elem->next;
}
