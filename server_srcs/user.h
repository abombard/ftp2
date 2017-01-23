#ifndef USER_H
# define USER_H

# include "sys_types.h"
# include "list.h"
# include "fifo.h"
# include "data.h"

# define USER_NAME_SIZE_MAX		63
# define PATH_SIZE_MAX			255

typedef struct	s_user
{
	t_list		fifo;
	bool		online;
	int			sock;
	char		name[USER_NAME_SIZE_MAX + 1];
	char		home[PATH_SIZE_MAX + 1];
	char		pwd[PATH_SIZE_MAX + 1];
	t_buf		msg_in;
	t_buf		msg_out;
}				t_user;

t_user	*user_new(t_fifo *fifo);
void	user_del(t_fifo *fifo, t_user *user);
void	user_show(t_user *user);

#endif
