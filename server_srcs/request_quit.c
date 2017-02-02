#include "server.h"
#include "strerror.h"
#include <sys/socket.h>

extern int		request_quit(int argc, char **argv, t_user *user, t_io *io)
{
	(void)argv;
	(void)user;
	if (argc > 1)
		return (E2BIG);
	send(io->sock, MSG(SUCCESS "\n"), MSG_DONTWAIT);
	delete_io(io);
	return (0);
}
