#include "get_next_line.h"
#include <unistd.h>
#include <stdio.h>

extern void	gnl_init(t_gnl *s, int fd, char *buf, size_t buf_size)
{
	s->fd = fd;
	s->buf = buf;
	s->buf_size = buf_size;
	s->size = 0;
	s->size_left = 0;
}

static bool	gnl_read(t_gnl *s)
{
	ssize_t	nb_read;

	nb_read = read(s->fd, s->buf + s->size_left, s->buf_size - s->size_left);
	if (nb_read < 0)
	{
		perror("read");
		return (false);
	}
	s->size_left += (size_t)nb_read;
	if (s->size_left == 0)
		return (false);
	return (true);
}

extern bool	get_next_line(t_gnl *s, t_buf *line)
{
	char	*pt;

	line->bytes = s->buf;
	line->size = 0;
	ft_memmove(s->buf, s->buf + s->size, s->size_left);
	pt = ft_memchr(s->buf, '\n', s->size_left);
	if (pt == NULL)
	{
		if (!gnl_read(s))
			return (false);
		pt = ft_memchr(s->buf, '\n', s->size_left);
		if (pt == NULL)
		{
			if (s->size_left == s->buf_size)
			{
				ft_fprintf(2, "Line too big\n");
				return (false);
			}
			s->size = s->size_left;
			s->size_left = 0;
			line->size = s->size;
			return (true);
		}
	}
	s->size = (size_t)(pt + 1 - s->buf);
	s->size_left -= s->size;
	line->size = s->size - 1;
	return (true);
}
