#ifndef SET_H
# define SET_H

# include "list.h"
# include <sys/select.h>

typedef struct	s_set
{
	t_list		datas;
	fd_set		fds;
}				t_set;

void	sets_teardown(t_set *sets);
int		sets_prepare(int listen, t_set *sets);

#endif
