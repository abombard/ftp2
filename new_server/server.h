#ifndef SERVER_H
# define SERVER_H

# include "sys_types.h"
# include "list.h"

# include <sys/select.h>

# define SUCCESS	"SUCCESS"
# define ERROR		"ERROR"

/*
** msg
*/
# define MSG_SIZE_MAX	255

typedef enum	e_msg_type
{
	MSG_TYPE_UNDEFINED,
	MSG_TYPE_MSG,
	MSG_TYPE_FILE
}				t_msg_type;

typedef struct	s_msg
{
	t_list		fifo;

	t_msg_type	type;
	int			fd;

	char		*bytes;
	size_t		offset;
	size_t		size;
	size_t		size_max;

}				t_msg;

/*
** io
*/
typedef struct	s_io
{
	t_list	list;

	bool	online;
	int		sock;
	t_msg	*in;
	t_list	msgs_out;
}				t_io;

/*
** user
*/
# define NAME_SIZE_MAX	63
# define PATH_SIZE_MAX	255

typedef struct	s_user
{
	char	name[NAME_SIZE_MAX + 1];
	char	home[PATH_SIZE_MAX + 1];
	char	pwd[PATH_SIZE_MAX + 1];

	t_io	io;

}				t_user;

/*
** select
*/
# define WFDS		0
# define RFDS		1
# define EFDS		2
# define SET_COUNT	3

typedef struct	s_select
{
	t_list	ios;
	fd_set	fds[SET_COUNT];
}				t_select;

/*
** server
*/
# define USER_COUNT_MAX		63

typedef struct	s_server
{
	int			listen;
	char		home[PATH_SIZE_MAX + 1];
	t_user		users[USER_COUNT_MAX];
	t_select	select;
}				t_server;

#endif
