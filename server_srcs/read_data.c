#include "server.h"
#include "strerror.h"

static bool	int_read_data(int sock, t_data *data)
{
	ssize_t		nbytes;

	nbytes = read(sock, data->bytes + data->size, data->size_max - data->size);
	if (nbytes < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return (true);
		perror("read", errno);
		return (false);
	}
	if (nbytes == 0 && data->size != data->size_max)
		return (false);
	data->size += (size_t)nbytes;
	return (true);
}

extern bool	read_data(t_server *server, t_io *io)
{
	t_data	*msg;

	if (!FD_ISSET(io->sock, &server->fds[RFDS]))
		return (true);
	if (!int_read_data(io->sock, &io->data_in))
	{
		delete_io(io);
		return (false);
	}
	if (io->data_in.size == io->data_in.size_max)
	{
		if (io->data_in.fd == -1)
		{
			msg = msg_error(strerror(EMSGSIZE));
			if (msg)
				list_add_tail(&msg->list, &io->datas_out);
		}
		else
			free_file(&io->data_in);
		io_input_teardown(io);
	}
	return (true);
}
