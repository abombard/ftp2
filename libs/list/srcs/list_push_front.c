#include "list.h"

void	list_push_front(t_list *new, t_list *head)
{
	t_list_add(new, head, head->next);
}
