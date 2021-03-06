#include "get_next_line.h"
#include "libft.h"
#include "strerror.h"

#include <sys/types.h>
#include <stdbool.h>
#include <stdlib.h>

static int	concat(t_gnl *gnl, char *s)
{
	char	*new;

	new = (char *)malloc(gnl->size + gnl->nread + 1);
	if (!new)
		return (errno);
	ft_memcpy(new, gnl->bytes, gnl->size);
	ft_memcpy(new + gnl->size, s, gnl->nread);
	free(gnl->bytes);
	gnl->bytes = new;
	gnl->size += gnl->nread;
	return (ESUCCESS);
}

static bool	get_line(t_gnl *gnl, t_buf *line)
{
	char	*pt;

	line->bytes = NULL;
	line->size = 0;
	pt = ft_memchr(gnl->bytes, '\n', gnl->size);
	if (pt)
	{
		line->bytes = gnl->bytes;
		line->size = (size_t)(pt - gnl->bytes);
		gnl->line_size = line->size + 1;
	}
	else if (gnl->nread == 0)
	{
		line->bytes = gnl->bytes;
		line->size = gnl->size;
		gnl->line_size = gnl->size;
	}
	return (line->bytes);
}

extern bool	get_next_line(t_gnl *gnl, t_buf *line)
{
	char	buf[512];
	int		err;

	gnl_flush_line(gnl);
	while (!get_line(gnl, line))
	{
		gnl->nread = read(gnl->fd, buf, sizeof(buf));
		if (gnl->nread < 0 || (gnl->nread == 0 && gnl->size == 0))
		{
			free(gnl->bytes);
			return (false);
		}
		err = concat(gnl, buf);
		if (err)
		{
			perror("concat", err);
			return (false);
		}
	}
	return (true);
}
