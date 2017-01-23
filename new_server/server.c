#include "server.h"

/*
** utils
*/
#include "printf.h"
#include "strerror.h"
void	perror(char *s, int err)
{
	ft_fprintf(2, "%s: %s\n", s, strerror(err));
}

/*
** server
*/
void		get_server(t_server **server_ptr)
{
	static t_server	server = {
		.listen = -1
	};

	*server_ptr = &server;
}

/*
** datas
*/
#include "list.h"
t_data		*data_pull(int fd, char *bytes, size_t size, size_t size_max)
{
	t_server	*server;
	t_data		*data;
	t_list		*pos;

	get_server(&server);
	if (list_is_empty(&server->data_fifo))
		return (NULL);
	pos = list_nth(&server->data_fifo, 1);
	data = CONTAINER_OF(pos, t_data, list);
	data->fd = fd;
	data->bytes = bytes;
	data->offset = 0;
	data->size = size;
	data->size_max = size_max;
	return (data);
}

void		data_store(t_data *data)
{
	t_server	*server;

	get_server(&server);
	list_add(&data->list, &server->data_fifo);
}

/*
** io
*/
t_io		*get_io(int sock)
{
	t_server	*server;

	get_server(&server);
	if (sock < 0 || (unsigned long)sock >= sizeof(server->io_array) / sizeof(server->io_array[0]))
		return (NULL);
	return (&server->io_array[sock]);
}

void		create_io(int sock)
{
	t_io	*io;

	io = get_io(sock);
	if (!io)
		return ;
}

void		delete_io(int sock)
{
	t_io	*io;

	io = get_io(sock);
	if (!io)
		return ;
}

/*
** sets
*/
static void	fds_set(t_list *io_list, fd_set *fds, int *nfds)
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

void		sets_prepare(int *nfds)
{
	t_server	*server;

	get_server(&server);

	FD_ZERO(&server->fds[RFDS]);
	FD_ZERO(&server->fds[WFDS]);
	FD_ZERO(&server->fds[EFDS]);

	FD_SET(server->listen, &server->fds[RFDS]);
	*nfds = server->listen + 1;

	fds_set(&server->io_list, server->fds, nfds);
}

/*
** server open / close
*/
#include "listen_socket.h"
#include "sock.h"
bool	server_open(char *host, int port)
{
	t_server	*server;
	t_io		*io;
	size_t		i;

	get_server(&server);
	server->host = host;
	server->port = port;
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

bool	server_close(void)
{
	t_server	*server;

	get_server(&server);
	close_socket(server->listen);
	return (true);
}

/*
** server loop
*/
bool	fds_availables(int nfds, int *ready)
{
	t_server		*server;
	struct timeval	tv;

	get_server(&server);

	tv.tv_sec = 10;
	tv.tv_usec = 0;

	*ready = select(
		nfds,
		&server->fds[RFDS],
		&server->fds[WFDS],
		&server->fds[EFDS],
		&tv
	);
	if (*ready == -1)
	{
		if (errno == EINTR)
			return (true);
		perror("select", errno);
		return (false);
	}

	return (true);
}

bool	handle_new_connections(void)
{
	t_server	*server;
	t_io		*io;
	int			sock;
	ssize_t		size;

	get_server(&server);
	if (!FD_ISSET(server->listen, &server->fds[RFDS]))
		return (true);
	sock = accept_connection(server->listen);
	if (sock == -1)
		return (false);
	io = get_io(sock);
	if (io == NULL)
	{
		size = send(sock, MSG("ERROR Too many connections\n"), MSG_DONTWAIT);
		if (size == -1)
			perror("send", errno);
		close_socket(sock);
		return (false);
	}
	list_add_tail(&io->list, &server->io_list);
	LOG_DEBUG("new user connected");
	return (true);
}


bool	int_recv_data(int sock, t_data *data)
{
	data->nbytes = recv(sock, data->bytes + data->size, data->size_max - data->size, MSG_DONTWAIT);
	if (data->nbytes < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return (true);
		perror("recv", errno);
		return (false);
	}
	LOG_DEBUG("recv {%.*s}", (int)data->nbytes, data->bytes + data->size);
	data->size += (size_t)data->nbytes;
	return (true);
}

bool	recv_data(t_io *io)
{
	t_server	*server;

	get_server(&server);
	if (!FD_ISSET(io->sock, &server->fds[RFDS]))
		return (true);
	return (int_recv_data(io->sock, &io->data_in));
}

bool	int_send_data(int sock, t_data *data)
{
	data->nbytes = send(sock, data->bytes + data->offset, data->size - data->offset, MSG_DONTWAIT);
	if (data->nbytes < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return (true);
		perror("send", errno);
		return (false);
	}
	LOG_DEBUG("send {%.*s}", (int)data->nbytes, data->bytes + data->offset);
	data->offset += (size_t)data->nbytes;
	return (true);
}

bool	send_data(t_io *io)
{
	t_server	*server;
	t_data		*data;
	t_list		*pos;

	get_server(&server);
	if (!FD_ISSET(io->sock, &server->fds[WFDS]))
		return (true);
	pos = list_nth(&io->datas_out, 1);
	data = CONTAINER_OF(pos, t_data, list);
	return (int_send_data(io->sock, data));
}

bool	foreach_io(bool (*io_func)(t_io *))
{
	t_server	*server;
	t_io		*io;
	t_list		*pos;
	int			nfails;

	get_server(&server);
	nfails = 0;
	pos = &server->io_list;
	while ((pos = pos->next) != &server->io_list)
	{
		io = CONTAINER_OF(pos, t_io, list);
		if (!io_func(io))
			nfails++;
	}
	return (nfails ? false : true);
}

bool	server_loop(void)
{
	int				nfds;
	int				ready;

	sets_prepare(&nfds);

	if (!fds_availables(nfds, &ready))
		return (false);
	if (ready == 0)
		return (true);

	if (!handle_new_connections())
		return (false);

	foreach_io(&recv_data);
	foreach_io(&send_data);

	return (true);
}

/*
** main
*/
#include "libft.h"
#include <unistd.h>
static volatile bool	run = true;

void	sighandler(int sig)
{
	(void)sig;
	run = false;
}

int		main(int argc, char **argv)
{
	char		*host;
	int			port;

	signal(SIGINT, sighandler);
	if (argc != 3)
	{
		ft_fprintf(2, "Usage: %s host port\n", argv[0]);
		return (EXIT_FAILURE);
	}
	host = argv[1];
	port = ft_atoi(argv[2]);
	if (!server_open(host, port))
	{
		LOG_ERROR("server_open failed host {%s} port {%s}", argv[1], argv[2]);
		return (EXIT_FAILURE);
	}
	while (run)
	{
		if (!server_loop())
		{
			LOG_ERROR("server_loop failed");
			break ;
		}
	}
	if (!server_close())
	{
		LOG_ERROR("server_close failed");
		return (EXIT_FAILURE);
	}
	return (run == false ? EXIT_SUCCESS : EXIT_FAILURE);
}
