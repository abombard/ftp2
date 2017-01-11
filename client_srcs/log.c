#include "log.h"
#include <unistd.h>			/* write() */
#include <stdarg.h>			/* va_start(), va_end() */
#include <stdio.h>			/* snprintf(), vsnprintf() */

bool	write_data (const int fd, const char *cmd, const size_t size)
{
	size_t	written;
	ssize_t	nwritten;

	written = 0;
	while (written < size)
	{
		nwritten = write (fd, cmd + written, size - written);
		if (nwritten < 0)
		{
			perror ("write");
			return ( false );
		}
		written += (size_t)nwritten;
	}
	return ( true );
}

void	private_log(const char *file,
					const char *func,
					const int line,
					const char *logtype,
					const char *fmt, ...)
{
	va_list		ap;
	int			size;
	int			buffer_size;
	char		buffer[LOG_MSG_SIZE_MAX + 1];

	buffer_size = 0;
	size = snprintf (buffer, LOG_MSG_SIZE_MAX, "%s:%s:%d:%s():", logtype, file, line, func);
	if (size < 0)
	{
		perror ("snprintf");
		return ;
	}
	else if ((unsigned int)size > LOG_MSG_SIZE_MAX)
	{
		fprintf (stderr, "snprintf size %d sizeof(buffer) %d", size, (int)LOG_MSG_SIZE_MAX);
		return ;
	}
	else
	{
		buffer_size += size;
	}
	va_start (ap, fmt);
	size = vsnprintf (buffer + buffer_size, LOG_MSG_SIZE_MAX - buffer_size, fmt, ap);
	va_end (ap);
	if (size < 0)
	{
		perror ("vsnprintf");
	}
	else if (buffer_size + size > LOG_MSG_SIZE_MAX)
	{
		fprintf (stderr, "vsnprintf size %d LOG_MSG_SIZE_MAX %d", buffer_size + size, (int)LOG_MSG_SIZE_MAX);
		return ;
	}
	else
	{
		buffer_size += size;
		buffer[buffer_size] = '\n';
		buffer_size += 1;
	}
	write_data(2, buffer, (size_t)buffer_size);
}

