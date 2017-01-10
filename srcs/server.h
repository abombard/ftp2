#ifndef FTP_H
# define FTP_H

# define DEBUG 1
# define EASY_DEBUG_IN if (DEBUG) fprintf(stderr, "in %s\n", __func__);
# define EASY_DEBUG_OUT if (DEBUG) fprintf(stderr, "out %s\n", __func__);

# include "sys_types.h"
# include "list.h"
# include "sock.h"
# include "listen_socket.h"

# include <stdio.h>

# define FTP_SUCCESS	"SUCCESS"
# define FTP_ERROR		"ERROR"

# define FTP_ERROR_INVALID_ARGUMENT	MSG(" Invalid Argument")

# define USER_NAME_SIZE_MAX		63

# define PATH_SIZE_MAX			255

typedef enum	e_fds
{
	NOFDS = 0,
	RFDS,
	WFDS,
	EFDS
}				t_fds;

# define MSG_SIZE_MAX	1023

typedef struct	s_data
{
	int		fd;
	char	*bytes;
	size_t	offset;
	size_t	size;
}				t_data;

# define MSG(str) (str), (sizeof(str) - 1)

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

typedef struct	s_set
{
	t_list		users;
	fd_set		fds;
}				t_set;

# define USER_COUNT_MAX		64

typedef struct	s_fifo
{
	t_list	users;
	t_list	datas;
}				t_fifo;

typedef struct	s_ftp
{
	int		sock_listen;

	t_fifo	fifo;

	t_set		sets[4];

	char		home[PATH_SIZE_MAX + 1];

}				t_ftp;

#endif
