#include "list.h"

void	list_add(t_list *new, t_list *head)
{
	t_list_add(new, head, head->next);
}
