#include "list.h"

void	list_insert(t_list *new, t_list *head, unsigned int index)
{
	t_list	*pos;

	pos = list_nth(head, index);
	list_add(new, pos);
}
