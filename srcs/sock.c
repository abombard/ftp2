#include "sock.h"
#include <stdio.h>	/* perror() */

SOCKET socket_open(const int ai_family,
			const int ai_socktype,
			const int ai_protocol)
{
	SOCKET	sock;

	sock = socket(ai_family, ai_socktype, ai_protocol);
	if (sock == INVALID_SOCKET)
		perror("socket");
	return (sock);
}

void socket_close(SOCKET sock)
{
	if (sock < 0)
		return ;
	shutdown(sock, SHUT_RDWR);
	close(sock);
}
