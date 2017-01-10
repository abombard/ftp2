#include "list.h"

void	list_splice(t_list *list, t_list *head)
{
	if (!list_is_empty(list))
	{
		t_list_splice(list, head);
		INIT_LIST_HEAD(list);
	}
}
