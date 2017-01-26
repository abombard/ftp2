#ifndef SERVER_H
# define SERVER_H

# include "sys_types.h"
# include "list.h"

# include <sys/select.h>

# define SUCCESS				"SUCCESS"
# define SUCCESS_SIZE	(sizeof("SUCCESS") - 1)
# define ERROR					"ERROR"
# define ERROR_SIZE		(sizeof("ERROR") - 1)

# define MSG(msg)	(msg), (sizeof(msg) - 1)

/*
** data
*/
typedef struct	s_data
{
	t_list		list;

	int			fd;
	char		*bytes;
	size_t		offset;
	size_t		size;
	size_t		size_max;

	ssize_t		nbytes;
}				t_data;

/*
** io
*/
typedef struct	s_io
{
	t_list	list;

	bool	connected;
	int		sock;
	t_data	data_in;
	char	*input_buffer;
	t_list	datas_out;
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
}				t_user;

/*
** server
*/
# define CONNECTION_COUNT_MAX		63
# define DATA_COUNT_MAX				(CONNECTION_COUNT_MAX * 2)
# define INPUT_BUFFER_SIZE			255

# define WFDS		0
# define RFDS		1
# define EFDS		2
# define SET_COUNT	3

typedef struct	s_server
{
	char	*host;
	int		port;

	int		listen;

	int		sig_warn;

	t_list	io_list;
	t_io	io_array[CONNECTION_COUNT_MAX];

	t_user	user_array[CONNECTION_COUNT_MAX];

	fd_set	fds[SET_COUNT];

	char	home[PATH_SIZE_MAX + 1];
}				t_server;

#endif
