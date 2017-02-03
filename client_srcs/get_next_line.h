#ifndef GET_NEXT_LINE_H
# define GET_NEXT_LINE_H

# include "buf.h"

# include <sys/types.h>
# include <stdbool.h>

typedef struct	s_gnl
{
	int		fd;
	ssize_t	nread;
	char	*bytes;
	size_t	size;
	size_t	line_size;
}				t_gnl;

void			gnl_init(t_gnl *gnl, int fd);
bool			get_next_line(t_gnl *gnl, t_buf *line);
bool			gnl_flush(t_gnl *gnl, t_buf *buf);
void			gnl_flush_line(t_gnl *gnl);

#endif
