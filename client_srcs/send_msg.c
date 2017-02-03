#include "strerror.h"
#include <unistd.h>
#include <sys/socket.h>

int		send_msg(const int sock, char *msg, size_t size)
{
	size_t	nsend;
	ssize_t	nbyte;

	nsend = 0;
	while (nsend < size)
	{
		nbyte = send(sock, msg, size, 0);
		if (nbyte < 0)
			return (errno);
		nsend += (size_t)nbyte;
	}
	return (ESUCCESS);
}
