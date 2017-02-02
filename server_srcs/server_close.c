#include "server.h"
#include "sock.h"

extern bool		server_close(t_server *server)
{
	close_socket(server->listen);
	return (true);
}
