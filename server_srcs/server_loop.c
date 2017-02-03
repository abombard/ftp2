#include "server.h"
#include "strerror.h"

static bool	fds_availables(int nfds, t_server *server, int *ready)
{
	struct timeval	tv;

	tv.tv_sec = 10;
	tv.tv_usec = 0;
	*ready = select(
		nfds,
		&server->fds[RFDS],
		&server->fds[WFDS],
		NULL,
		&tv);
	if (*ready == -1)
	{
		if (errno != EINTR)
			return (false);
	}
	return (true);
}

extern bool	server_loop(t_server *server)
{
	int		nfds;
	int		ready;
	bool	new_user;

	sets_prepare(server, &nfds);
	if (!fds_availables(nfds, server, &ready))
		return (false);
	if (ready == 0)
		return (true);
	if (!handle_new_connections(server, &new_user))
		return (false);
	if (new_user)
		return (true);
	foreach_io(server, &send_data);
	foreach_io(server, &read_data);
	foreach_io(server, &request);
	return (true);
}
