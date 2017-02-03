#include "client.h"
#include "strerror.h"
#include "libft.h"
#include "sock.h"
#include "get_next_line.h"

#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

int		simple_output(t_gnl *gnl_sock, t_buf *msg)
{
	if (!get_next_line(gnl_sock, msg))
		return (ECONNABORTED);
	write_data(1, msg->bytes, msg->size);
	write_data(1, "\n", sizeof("\n") - 1);
	return (0);
}

#define PROMPT		"$> "

int		client_loop(const int sock)
{
	t_gnl	gnl_stdin;
	t_gnl	gnl_sock;
	t_buf	msg;
	int		err;

	gnl_init(&gnl_sock, sock);
	gnl_init(&gnl_stdin, 0);
	while (write_data(1, PROMPT, sizeof(PROMPT) - 1), get_next_line(&gnl_stdin, &msg))
	{
		if (!ft_memcmp(msg.bytes, "quit", sizeof("quit") - 1))
			break ;
		if (islocal(&msg))
			local(&msg);
		else if (isfiletransfer(&msg))
		{
			if ((err = init_file_transfer(sock, &gnl_sock, &msg)))
				return (err);
		}
		else
		{
			if ((err = send_msg(sock, msg.bytes, msg.size + 1)) ||
				(err = simple_output(&gnl_sock, &msg)))
				return (err);
		}
	}
	return (0);
}

int		main(int argc, char **argv)
{
	int		sock;
	char	*host;
	int		port;
	int		err;

	if (argc != 3)
	{
		ft_fprintf(2, "Usage: %s server port\n", argv[0]);
		return (1);
	}
	host = argv[1];
	port = ft_atoi(argv[2]);
	sock = query_connection(host, port);
	if (sock == -1)
		return (1);
	err = client_loop(sock);
	if (err && err != ECONNABORTED)
		perror("client_loop", err);
	close(sock);
	return (err);
}
