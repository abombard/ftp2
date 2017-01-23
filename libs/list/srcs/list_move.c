#include "list.h"

void	list_move(t_list *list, t_list *head)
{
	t_list_del(list->prev, list->next);
	list_add(list, head);
}

void	list_move_tail(t_list *list, t_list *head)
{
	t_list_del(list->prev, list->next);
	list_add_tail(list, head);
}
