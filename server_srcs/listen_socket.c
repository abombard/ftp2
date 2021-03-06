#include "sock.h"
#include "strerror.h"
#include "libft.h"
#include "printf.h"

#include <sys/socket.h>
#include <netdb.h>
#include <stdbool.h>
#include <string.h>

static bool		addrinfo_alloc(const char *host, struct addrinfo **out_result)
{
	struct addrinfo	*result;
	struct addrinfo	hints;
	int				err;

	*out_result = NULL;
	ft_memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;
	err = getaddrinfo(host, "ftp", &hints, &result);
	if (err)
	{
		ft_fprintf(2, "getaddrinfo: %s\n", gai_strerror(err));
		return (false);
	}
	*out_result = result;
	return (true);
}

#define SOCKET_BACKLOG_COUNT_MAX	1

static int		sock_bind(const int sock, const int port)
{
	struct sockaddr_in	sockaddr;
	int					yes;

	yes = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
		return (errno);
	ft_memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_port = htons(port);
	sockaddr.sin_family = AF_INET;
	if (bind(sock, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) == -1)
		return (errno);
	if (listen(sock, SOCKET_BACKLOG_COUNT_MAX))
		return (errno);
	return (ESUCCESS);
}

extern int		listen_socket(const char *host, const int port)
{
	struct addrinfo *result;
	int				sock;
	struct addrinfo *rp;
	int				err;

	sock = -1;
	if (!addrinfo_alloc(host, &result))
		return (-1);
	rp = result;
	while (rp != NULL)
	{
		sock = open_socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sock != -1)
		{
			if (!(err = sock_bind(sock, port)))
				break ;
			close_socket(sock);
			sock = -1;
		}
		rp = rp->ai_next;
	}
	freeaddrinfo(result);
	return (sock);
}
