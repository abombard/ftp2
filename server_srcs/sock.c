#include "sock.h"
#include "libft.h"
#include "strerror.h"
#include "printf.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define INET_NTOA(p)	inet_ntoa(*((struct in_addr *)p))

int		open_socket(const int ai_family,
					const int ai_socktype,
					const int ai_protocol)
{
	int	sock;

	sock = socket(ai_family, ai_socktype, ai_protocol);
	if (sock == -1)
		perror("socket", errno);
	return (sock);
}

void	close_socket(int sock)
{
	if (sock < 0)
		return ;
	shutdown(sock, SHUT_RDWR);
	close(sock);
}

int		query_connection(char *host, int port)
{
	int					sock;
	struct sockaddr_in	sockaddr;
	struct hostent		*server_info;

	ft_memset(&sockaddr, 0, sizeof(sockaddr));
	if ((server_info = gethostbyname(host)) == NULL)
	{
		herror("gethostbname");
		return (-1);
	}
	sockaddr.sin_addr.s_addr = inet_addr(INET_NTOA(server_info->h_addr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	if ((sock = socket(sockaddr.sin_family, SOCK_STREAM, 0)) == -1)
	{
		perror("socket", errno);
		return (-1);
	}
	if (connect(sock, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr)))
	{
		perror("connect", errno);
		close(sock);
		return (-1);
	}
	return (sock);
}

int		accept_connection(int listen_socket)
{
	int					sock;
	socklen_t			addr_size;
	struct sockaddr_in	addr;

	addr_size = sizeof(addr);
	sock = accept(listen_socket, (struct sockaddr *)&addr, &addr_size);
	if (sock == -1)
	{
		perror("accept", errno);
		return (-1);
	}
	if (addr_size > sizeof(addr))
	{
		ft_fprintf(2, "ERROR: addr_size %zu sizeof(addr) %zu",
				(size_t)addr_size, (size_t)sizeof(addr));
		close(sock);
		return (-1);
	}
	return (sock);
}
