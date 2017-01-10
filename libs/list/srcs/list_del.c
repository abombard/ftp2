#include "list.h"

void	list_del(t_list *entry)
{
	t_list_del(entry->prev, entry->next);
	INIT_LIST_HEAD(entry);
}
