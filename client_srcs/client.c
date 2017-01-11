#include "client.h"
#include "get_next_line.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <stdio.h>

int		service_create(const char *server_name, const int port)
{
	int					sock;
	struct sockaddr_in	sockaddr;
	struct hostent		*server_info;

	ft_memset(&sockaddr, 0, sizeof(sockaddr));
	server_info = gethostbyname(server_name);
	if (server_info == NULL)
	{
		herror("gethostbname");
		return (-1);
	}
	sockaddr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)server_info->h_addr)));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);

	sock = socket(sockaddr.sin_family, SOCK_STREAM, 0);
	if (sock == -1)
	{
		perror("socket");
		return (-1);
	}

	if (connect(sock, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr)) < 0)
	{
		perror("connect");
		close(sock);
		return (-1);
	}

	return (sock);
}

#define MSG_SIZE_MAX	1023

bool	session_open(const int sock)
{
	char	buf[MSG_SIZE_MAX + 1];
	ssize_t	size;

	size = recv(sock, buf, MSG_SIZE_MAX, 0);
	if (size < 0)
	{
		perror("recv");
		return (false);
	}
	buf[size] = '\0';
	if (ft_memcmp(buf, "SUCCESS", sizeof("SUCCESS") - 1))
	{
		ft_fprintf(2, "Failed to connect\n");
		return (false);
	}
	return (true);
}

bool	client_loop(const int sock)
{
	t_gnl			gnl;
	t_buf			line;
	char			msg[MSG_SIZE_MAX + 1];
	ssize_t			nread;

	gnl_init(&gnl, 0);
	while (get_next_line(&gnl, &line))
	{
		fd_set			fds;
		int				ready;
		struct timeval	tv;

		// write
		FD_ZERO(&fds);
		FD_SET(sock, &fds);

		tv.tv_sec = 1000;
		tv.tv_usec = 0;

		ready = select(sock + 1, NULL, &fds, NULL, &tv);
		if (ready <= 0)
		{
			ft_fprintf(2, "ready %d", ready);
			break ;
		}

		if (send(sock, line.bytes, line.size, 0) != (ssize_t)line.size)
		{
			perror("send");
			return (false);
		}

		// read
		FD_ZERO(&fds);
		FD_SET(sock, &fds);

		tv.tv_sec = 1000;
		tv.tv_usec = 0;

		ready = select(sock + 1, &fds, NULL, NULL, &tv);
		if (ready <= 0)
		{
			ft_fprintf(2, "ready %d", ready);
			break ;
		}

		nread = recv(sock, msg, MSG_SIZE_MAX, 0);
		if (nread < 0)
		{
			perror("recv");
			return (false);
		}
		if (nread == 0)
			break ;

		msg[MSG_SIZE_MAX] = '\0';
		ft_printf("%s", msg);
	}

	return (true);
}

int		main(int ac, char **av)
{
	int		sock;
	char	*server_name;
	int		port;
	int		status;

	if (ac != 3)
	{
		ft_fprintf(2, "Usage: %s server port\n", av[0]);
		return (1);
	}
	server_name = av[1];
	port = ft_atoi(av[2]);
	sock = service_create(server_name, port);
	if (sock == -1)
		return (1);
	if (!session_open(sock))
	{
		ft_fprintf(2, "session_open failed\n");
		return (1);
	}
	status = client_loop(sock) ? 0 : 1;
	close(sock);
	return (status);
}
