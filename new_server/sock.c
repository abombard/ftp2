#include "sock.h"
#include "strerror.h"

int		accept_connection(int listen_socket)
{
	int					sock;
	socklen_t			addr_size;
	struct sockaddr_in	addr;

	addr_size = sizeof (addr);
	sock = accept(listen_socket, (struct sockaddr *)&addr, &addr_size);
	if (sock == -1)
	{
		LOG_ERROR("accept: %s", strerror(errno));
		return (-1);
	}
	if (addr_size > sizeof (addr))
	{
		LOG_ERROR("addr_size %zu sizeof(addr) %zu", (size_t)addr_size, (size_t)sizeof(addr));
		close(sock);
		return (-1);
	}
	return (sock);
}

int	socket_open(const int ai_family,
			const int ai_socktype,
			const int ai_protocol)
{
	int	sock;

	sock = socket(ai_family, ai_socktype, ai_protocol);
	if (sock == -1)
		LOG_ERROR("socket: %s", strerror(errno));
	return (sock);
}

void socket_close(int sock)
{
	if (sock < 0)
		return ;
	shutdown(sock, SHUT_RDWR);
	close(sock);
}
