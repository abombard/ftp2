#include "list.h"

int	list_is_empty(t_list *head)
{
	return (head->next == head);
}
