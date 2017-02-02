#include "server.h"
#include "sock.h"
#include "strerror.h"
#include <stdlib.h>

extern t_io		*get_io(int sock, t_server *server)
{
	if (sock < 0 || (unsigned long)sock >= sizeof(server->io_array) / sizeof(server->io_array[0]))
		return (NULL);
	return (&server->io_array[sock]);
}

extern void		io_input_teardown(t_io *io)
{
	io->data_in.fd = -1;
	io->data_in.bytes = io->input_buffer;
	io->data_in.size_max = INPUT_BUFFER_SIZE;
	io->data_in.size = 0;
	io->data_in.offset = 0;
}

extern int		create_io(int sock, t_server *server)
{
	t_io		*io;
	t_user		*user;

	io = get_io(sock, server);
	if (!io)
		return (EUSERS);
	clear_data(&io->data_in);
	io->input_buffer = (char *)malloc(INPUT_BUFFER_SIZE);
	if (!io->input_buffer)
		return (errno);
	io_input_teardown(io);
	INIT_LIST_HEAD(&io->datas_out);
	list_add_tail(&io->list, &server->io_list);
	io->connected = true;
	user = get_user(sock, server);
	user_init(user, server->home);
	return (0);
}

extern void		delete_io(t_io *io)
{
	t_data		*data;
	t_list		*pos;

	list_del(&io->list);
	if (io->data_in.fd != -1)
		free_file(&io->data_in);
	free(io->input_buffer);
	while (!list_is_empty(&io->datas_out))
	{
		pos = list_nth(&io->datas_out, 1);
		data = CONTAINER_OF(pos, t_data, list);
		free_data(data);
	}
	close_socket(io->sock);
	io->connected = false;
}

extern void	foreach_io(t_server *server, bool (*io_func)(t_server *, t_io *))
{
	t_io		*io;
	t_list		*pos;
	t_list		*safe;
	int			nfails;

	nfails = 0;
	safe = server->io_list.next;
	while ((pos = safe) != &server->io_list && (safe = pos->next))
	{
		io = CONTAINER_OF(pos, t_io, list);
		if (!io_func(server, io))
			nfails++;
	}
}
