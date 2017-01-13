#ifndef GET_NEXT_LINE_H
# define GET_NEXT_LINE_H

# include "deps.h"

typedef struct	s_gnl
{
	int		fd;
	char	*buf;
	size_t	buf_size;
	size_t	size;
	size_t	size_left;
}				t_gnl;

extern void		gnl_init(t_gnl *s, int fd, char *buf, size_t buf_size);
extern bool		get_next_line(t_gnl	*s, t_buf *line);

#endif
