#include "server.h"
#include "libft.h"
#include "log.h"

extern size_t	concat_safe(char *buf, size_t offset, size_t size_max, char *s)
{
	size_t	size;

	size = ft_strlen(s);
	if (offset + size >= size_max)
	{
		if ((int)(size_max - offset - 1) > 0)
		{
			size = size_max - offset - 1;
		}
		else
		{
			size = 0;
		}
		buf[offset + size - 1] = '$';
	}
	ft_memcpy(buf + offset, s, size);
	return (offset + size);
}

extern void		concat_path(char *pwd, char *in_path, char *path)
{
	if (in_path[0] == '/')
	{
		ft_strncpy(path, in_path, PATH_SIZE_MAX);
	}
	else
	{
		ft_strncpy(path, pwd, PATH_SIZE_MAX);
		ft_strncat(path, "/", PATH_SIZE_MAX);
		ft_strncat(path, in_path, PATH_SIZE_MAX);
	}
}
