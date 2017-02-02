#include "server.h"
#include "listen_socket.h"
#include "sock.h"
#include "libft.h"

bool	server_open(char *host, int port, char *home, t_server *server)
{
	t_io		*io;
	size_t		i;

	server->host = host;
	server->port = port;
	if (!home)
		home = "/tmp";
	ft_strncpy(server->home, home, PATH_SIZE_MAX);
	server->listen = listen_socket(host, port);
	if (server->listen == -1)
		return (false);
	INIT_LIST_HEAD(&server->io_list);
	i = 0;
	while (i < sizeof(server->io_array) / sizeof(server->io_array[0]))
	{
		io = &server->io_array[i];
		io->connected = false;
		io->sock = i;
		INIT_LIST_HEAD(&io->datas_out);
		i++;
	}
	return (true);
}
