#ifndef FTP_H
# define FTP_H

# define DEBUG 1
# define EASY_DEBUG_IN if (DEBUG) fprintf(stderr, "in %s\n", __func__);
# define EASY_DEBUG_OUT if (DEBUG) fprintf(stderr, "out %s\n", __func__);

# include "sys_types.h"
# include "list.h"
# include "sock.h"
# include "listen_socket.h"

# include "fifo.h"
# include "data.h"
# include "user.h"
# include "set.h"

# include <stdio.h>

# define FTP_SUCCESS	"SUCCESS"
# define FTP_ERROR		"ERROR"

# define USER_COUNT_MAX		64

typedef struct	s_ftp
{
	int		sock_listen;
	t_user	users[USER_COUNT_MAX];
	
	t_set	sets[4];
	char	home[PATH_SIZE_MAX + 1];
}				t_ftp;

#endif
