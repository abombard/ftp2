#ifndef SYS_TYPES_H
# define SYS_TYPES_H

# include <stdbool.h>
# include <inttypes.h>
# include <unistd.h>
# include <pthread.h>
# include <signal.h>

# define STR_TO_buf(str) { sizeof(str) - 1, str }

typedef struct	s_buf
{
	size_t	size;
	char	*bytes;
}				t_buf;

#endif
