#include "server.h"
#include "strerror.h"

static bool	int_send_data(int sock, t_data *data)
{
	ssize_t		nbytes;

	nbytes = write(sock, data->bytes + data->offset, data->size - data->offset);
	if (nbytes < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return (true);
		perror("send", errno);
		return (false);
	}
	data->offset += (size_t)nbytes;
	return (true);
}

extern bool	send_data(t_server *server, t_io *io)
{
	t_data		*data;
	t_list		*pos;
	bool		success;

	if (!FD_ISSET(io->sock, &server->fds[WFDS]))
		return (true);
	while (!list_is_empty(&io->datas_out))
	{
		pos = list_nth(&io->datas_out, 1);
		data = CONTAINER_OF(pos, t_data, list);
		success = int_send_data(io->sock, data);
		if (data->offset != data->size)
			break ;
		free_data(data);
	}
	return (success);
}
