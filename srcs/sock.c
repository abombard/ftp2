#include "sock.h"
#include <stdio.h>	/* perror() */

int	socket_open(const int ai_family,
			const int ai_socktype,
			const int ai_protocol)
{
	int	sock;

	sock = socket(ai_family, ai_socktype, ai_protocol);
	if (sock == -1)
		perror("socket");
	return (sock);
}

void socket_close(int sock)
{
	if (sock < 0)
		return ;
	shutdown(sock, SHUT_RDWR);
	close(sock);
}
