#ifndef FIFO_H
# define FIFO_H

# include "sys_types.h"
# include "list.h"

typedef struct	s_fifo
{
	t_list	users;
	t_list	datas;
}				t_fifo;

bool	fifo_empty(t_list *fifo);
void	*fifo_pull(t_list *fifo);
void	fifo_store(void *elem, t_list *fifo);
bool	fifo_create(size_t size, size_t count, t_list *fifo);
void	fifo_destroy(t_list *fifo);

#endif
