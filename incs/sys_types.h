#ifndef SYS_TYPES_H
# define SYS_TYPES_H

# include <stdbool.h>
# include <inttypes.h>
# include <unistd.h>
# include <pthread.h>
# include <signal.h>

# define STR_TO_BUFFER(str) { sizeof(str) - 1, str }

typedef struct	s_buffer
{
	size_t	size;
	char	*bytes;
}				t_buffer;

typedef struct	s_sig_warn
{
	sigset_t	sigset;
	int			fd;
}				t_sig_warn;

#endif
