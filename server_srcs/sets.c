#include "server.h"

static void		fds_set(t_list *io_list, fd_set *fds, int *nfds)
{
	t_io	*io;
	t_list	*pos;

	pos = io_list;
	while ((pos = pos->next) != io_list)
	{
		io = CONTAINER_OF(pos, t_io, list);
		FD_SET(io->sock, &fds[RFDS]);
		if (!list_is_empty(&io->datas_out))
			FD_SET(io->sock, &fds[WFDS]);
		if (io->sock >= *nfds)
			*nfds = io->sock + 1;
	}
}

extern void		sets_prepare(t_server *server, int *nfds)
{
	FD_ZERO(&server->fds[RFDS]);
	FD_ZERO(&server->fds[WFDS]);

	FD_SET(server->listen, &server->fds[RFDS]);
	*nfds = server->listen + 1;

	fds_set(&server->io_list, server->fds, nfds);
}
