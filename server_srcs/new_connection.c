#include "server.h"
#include "sock.h"
#include "strerror.h"
#include "libft.h"
#include <sys/socket.h>

extern bool	handle_new_connections(t_server *server, bool *new_user)
{
	int			sock;
	ssize_t		size;
	int			errnum;
	char		*err;

	*new_user = false;
	if (!FD_ISSET(server->listen, &server->fds[RFDS]))
		return (true);
	sock = accept_connection(server->listen);
	if (sock == -1)
		return (false);
	errnum = create_io(sock, server);
	if (errnum)
	{
		err = strerror(errnum);
		size = send(sock, err, ft_strlen(err), MSG_DONTWAIT);
		if (size == -1)
			perror("send", errno);
		close_socket(sock);
		return (false);
	}
	*new_user = true;
	return (true);
}
