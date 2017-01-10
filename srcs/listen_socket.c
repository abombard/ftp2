#include "sock.h"

#include <string.h>		/* memset() */
#include <stdio.h>		/* perror() */

static bool		addrinfo_alloc_init (const char *host,
						 			 struct addrinfo **out_result)
{
	struct addrinfo	*result;
	struct addrinfo	hints;
	int				err;

	*out_result = NULL;
	memset(&hints, 0, sizeof (hints));
	hints.ai_family = AF_UNSPEC;			/* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM;		/* Stream socket */
	hints.ai_flags = AI_PASSIVE;			/* For wildcard IP address */
	hints.ai_protocol = 0;					/* Any protocol */
	err = getaddrinfo(host, "ftp", &hints, &result);
	if (err)
		LOG_FATAL("getaddrinfo: %s", gai_strerror (err));
	*out_result = result;
	return (true);
}

#define SOCKET_BACKLOG_COUNT_MAX	1
static bool	sock_bind(const int sock, const int port)
{
	struct sockaddr_in	sockaddr;
	int					yes;

	yes = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (yes)) == -1)
		return (false);
	memset(&sockaddr, 0, sizeof (sockaddr));
	sockaddr.sin_port = htons(port);
	sockaddr.sin_family = AF_INET;
	if (bind(sock, (struct sockaddr *)&sockaddr, sizeof (sockaddr)) == -1)
		return (false);
	LOG_DEBUG("accepting connections on port %d\n", port);
	if (listen(sock, SOCKET_BACKLOG_COUNT_MAX))
		return (false);
	return (true);
}

int			listen_socket(const char *host, const int port)
{
	struct addrinfo *result;
	int				sock;
	struct addrinfo *rp;

	if (!addrinfo_alloc_init(host, &result))
	{
		LOG_ERROR("addrinfo_alloc_init failed");
		return (-1);
	}
	rp = result;
	while (rp != NULL)
	{
		sock = socket_open(rp->ai_family,
							rp->ai_socktype,
							rp->ai_protocol);
		if (sock != -1)
		{
			if (sock_bind(sock, port))
				break ;
			socket_close(sock);
		}
		rp = rp->ai_next;
	}
	if (rp == NULL)
		sock = -1;
	freeaddrinfo(result);
	return (sock);
}
