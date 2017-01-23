#ifndef SERVER_H
# define SERVER_H

# include "sys_types.h"
# include "list.h"

# include <sys/select.h>

# define SUCCESS	"SUCCESS"
# define ERROR		"ERROR"

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
	t_list	datas_out;
}				t_io;

/*
** server
*/
# define CONNECTION_COUNT_MAX		63
# define DATA_COUNT_MAX				(CONNECTION_COUNT_MAX * 2)

# define WFDS		0
# define RFDS		1
# define EFDS		2
# define SET_COUNT	3

typedef struct	s_server
{
	char	*host;
	int		port;

	int		listen;

	t_list	data_fifo;
	t_data	data_array[DATA_COUNT_MAX];

	t_list	io_list;
	t_io	io_array[CONNECTION_COUNT_MAX];

	fd_set	fds[SET_COUNT];
}				t_server;

#endif
