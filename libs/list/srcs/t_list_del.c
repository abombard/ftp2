#include "list.h"

void	t_list_del(t_list *prev, t_list *next)
{
	prev->next = next;
	next->prev = prev;
}
