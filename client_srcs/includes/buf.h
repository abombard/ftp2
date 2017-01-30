#ifndef BUF_H
# define BUF_H

# include <sys/types.h>

typedef struct	s_buf
{
	char	*bytes;
	size_t	size;
}				t_buf;

#endif
