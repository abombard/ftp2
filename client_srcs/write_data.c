#include <unistd.h>
#include "strerror.h"

int		write_data(const int fd, const char *cmd, const unsigned int size)
{
	size_t	written;
	ssize_t	nwritten;

	written = 0;
	while (written < size)
	{
		nwritten = write(fd, cmd + written, size - written);
		if (nwritten < 0)
		{
			perror("write", errno);
			return (0);
		}
		written += (size_t)nwritten;
	}
	return ((int)written);
}
