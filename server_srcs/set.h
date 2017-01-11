#ifndef SET_H
# define SET_H

# include "user.h"

typedef struct	s_set
{
	t_list		users;
	fd_set		fds;
}				t_set;

void	sets_teardown(t_set *sets);
int		sets_prepare(int listen, t_set *sets);
void	set_move(t_user *user, t_set *sets);

#endif
