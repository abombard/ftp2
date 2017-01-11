#include "sys_types.h"
#include "list.h"

#include <stdlib.h>

bool	fifo_empty(t_list *fifo)
{
	return (list_is_empty(fifo));
}

void	*fifo_pull(t_list *fifo)
{
	t_list	*elem;

	if (list_is_empty(fifo))
		return (NULL);
	elem = list_nth(fifo, 1);
	list_del(elem);
	return ((void *)elem);
}

void	fifo_store(void *elem, t_list *fifo)
{
	t_list	*pos;

	pos = (t_list *)elem;
	list_push_front(pos, fifo);
}

bool	fifo_create(size_t size, size_t count, t_list *fifo)
{
	t_list	*elem;
	size_t	i;

	INIT_LIST_HEAD(fifo);
	i = 0;
	while (i < count)
	{
		elem = (void *)malloc(size);
		if (elem == NULL)
			return (false);
		fifo_store(elem, fifo);
		i++;
	}
	return (true);
}

void	fifo_destroy(t_list *fifo)
{
	t_list	*elem;

	while (!list_is_empty(fifo))
	{
		elem = list_nth(fifo, 1);
		list_del(elem);
		free(elem);
	}
}
