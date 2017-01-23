#include "list.h"

void	list_add_tail(t_list *new, t_list *head)
{
	t_list_add(new, head->prev, head);
}
