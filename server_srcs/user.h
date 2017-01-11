#ifndef USER_H
# define USER_H

# include "sys_types.h"
# include "list.h"
# include "fifo.h"
# include "data.h"

# define USER_NAME_SIZE_MAX		63
# define PATH_SIZE_MAX			255

typedef enum	e_fds
{
	NOFDS = 0,
	RFDS,
	WFDS,
	EFDS
}				t_fds;

typedef struct	s_user
{
	bool		used;
	int			sock;
	char		name[USER_NAME_SIZE_MAX + 1];
	char		home[PATH_SIZE_MAX + 1];
	char		pwd[PATH_SIZE_MAX + 1];
	t_fds		fds;
	t_fds		nextfds;
	t_data		data;
	t_list		set;
}				t_user;

char	*fds_tostring(t_fds fds);
t_user	*user_new(t_fifo *fifo);
void	user_del(t_fifo *fifo, t_user *user);
void	user_show(t_user *user);

#endif
