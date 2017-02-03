#include "get_next_line.h"
#include "libft.h"

extern void	gnl_flush_line(t_gnl *gnl)
{
	ft_memmove(gnl->bytes, gnl->bytes + gnl->line_size,
				gnl->size - gnl->line_size);
	gnl->size -= gnl->line_size;
	gnl->line_size = 0;
}

extern void	gnl_init(t_gnl *gnl, int fd)
{
	gnl->fd = fd;
	gnl->nread = -1;
	gnl->bytes = NULL;
	gnl->size = 0;
	gnl->line_size = 0;
}

extern bool	gnl_flush(t_gnl *gnl, t_buf *buf)
{
	gnl_flush_line(gnl);
	buf->bytes = gnl->bytes;
	buf->size = gnl->size;
	gnl->line_size = gnl->size;
	return (true);
}
