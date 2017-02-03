#ifndef SYS_TYPES_H
# define SYS_TYPES_H

# include <stdbool.h>
# include <inttypes.h>
# include <unistd.h>
# include <pthread.h>
# include <signal.h>

typedef struct	s_buf
{
	size_t	size;
	char	*bytes;
}				t_buf;

#endif
