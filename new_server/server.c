#include "server.h"
#include "log.h"
#include "libft.h"

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
t_server	*get_server(void)
{
	static t_server	server = {
		.listen = -1
	};

	return (&server);
}

/*
** user
*/
t_user	*get_user(int sock)
{
	t_server	*server;

	server = get_server();
	if (sock < 0 || (unsigned long)sock >= sizeof(server->user_array) / sizeof(server->user_array[0]))
		return (NULL);
	return (&server->user_array[sock]);
}

/*
** datas
*/
bool		data_empty(void)
{
	t_server	*server;

	server = get_server();
	return (list_is_empty(&server->data_fifo));
}

void		data_clear(t_data *data)
{
	data->fd = -1;
	data->bytes = NULL;
	data->offset = 0;
	data->size = 0;
	data->size_max = 0;
}

#include "list.h"
t_data		*data_pull(void)
{
	t_server	*server;
	t_data		*data;
	t_list		*pos;

	server = get_server();
	if (list_is_empty(&server->data_fifo))
		return (NULL);
	pos = list_nth(&server->data_fifo, 1);
	data = CONTAINER_OF(pos, t_data, list);
	data_clear(data);
	return (data);
}

void		data_store(t_data *data)
{
	t_server	*server;

	server = get_server();
	list_add(&data->list, &server->data_fifo);
}

/*
** data msg
*/
#include <stdlib.h>
t_data		*data_pull_msg(size_t size)
{
	t_data	*data;

	data = data_pull();
	if (!data)
	{
		LOG_ERROR("data_pull failed");
		return (NULL);
	}
	data->bytes = (char *)malloc(size + 1);
	if (!data->bytes)
	{
		perror("malloc", errno);
		return (NULL);
	}
	data->size_max = size;
	return (data);
}

void		data_free_msg(t_data *data)
{
	free(data->bytes);
}

t_data		*msg_success(char *msg)
{
	t_data	*data;
	size_t	msg_size;

	msg_size = SUCCESS_SIZE + sizeof(" ") - 1 + ft_strlen(msg);
	data = data_pull_msg(msg_size);
	if (!data)
		return (NULL);
	ft_strncpy(data->bytes, SUCCESS, data->size_max);
	ft_strncat(data->bytes, " ", data->size_max);
	ft_strncat(data->bytes, msg, data->size_max);
	data->size = ft_strlen(data->bytes);
	return (data);
}

t_data		*msg_error(char *msg)
{
	t_data	*data;
	size_t	msg_size;

	msg_size = ERROR_SIZE + sizeof(" ") - 1 + ft_strlen(msg);
	data = data_pull_msg(msg_size);
	if (!data)
		return (NULL);
	ft_strncpy(data->bytes, ERROR, data->size_max);
	ft_strncat(data->bytes, " ", data->size_max);
	ft_strncat(data->bytes, msg, data->size_max);
	data->size = ft_strlen(data->bytes);
	return (data);
}

void		push_data(t_list *data_list, t_data *data)
{
	list_add_tail(&data->list, data_list);
}

/* //TEMP */
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
bool		data_create_file(t_data *data, char *file, size_t size)
{
	int		fd;
	char	*map;

	// What if size == 0 ?
	fd = open(file, O_CREAT | O_WRONLY, 0644);
	if (fd == -1)
	{
		perror("open", errno);
		return (false);
	}
	map = mmap(0, size, PROT_WRITE, MAP_PRIVATE, fd, 0);
	if (map == MAP_FAILED)
	{
		perror("mmap", errno);
		close(fd);
		return (false);
	}
	data->fd = fd;
	data->bytes = map;
	data->offset = 0;
	data->size = size;
	data->size_max = size;
	return (true);
}

bool		data_read_file(t_data *data, char *file)
{
	int			fd;
	struct stat	sb;
	char		*map;

	fd = open(file, O_RDONLY);
	if (fd == -1)
	{
		perror("open", errno);
		return (false);
	}
	if (fstat(fd, &sb))
	{
		perror("fstat", errno);
		close(fd);
		return (false);
	}
	// What if sb.st_size == 0 ?
	map = mmap(0, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (map == MAP_FAILED)
	{
		perror("mmap", errno);
		close(fd);
		return (false);
	}
	data->fd = fd;
	data->bytes = map;
	data->size = 0;
	data->size_max = sb.st_size;
	return (true);
}
/* //TEMP */

void		data_free_file(t_data *data)
{
	munmap(data->bytes, data->size_max);
	close(data->fd);
}

void		data_free(t_data *data)
{
	if (data->fd == -1)
		data_free_msg(data);
	else
		data_free_file(data);
}

/*
** data file
*/
//TEMP

/*
** io
*/
t_io		*get_io(int sock, t_server *server)
{
	if (sock < 0 || (unsigned long)sock >= sizeof(server->io_array) / sizeof(server->io_array[0]))
		return (NULL);
	return (&server->io_array[sock]);
}

#include "sock.h"
bool		create_io(int sock)
{
	t_server	*server;
	t_io		*io;

	server = get_server();
	io = get_io(sock, server);
	if (!io)
	{
		LOG_ERROR("get_io failed sock %d", sock);
		return (false);
	}
	data_clear(&io->data_in);
	io->data_in.bytes = (char *)malloc(MSG_SIZE_MAX + 1);
	if (!io->data_in.bytes)
	{
		perror("malloc", errno);
		return (false);
	}
	io->data_in.size_max = MSG_SIZE_MAX + 1;
	INIT_LIST_HEAD(&io->datas_out);
	list_add_tail(&io->list, &server->io_list);
	io->connected = true;
	return (true);
}

void		delete_io(t_io *io)
{
	t_data		*data;
	t_list		*pos;

	data_free(&io->data_in);
	while (!list_is_empty(&io->datas_out))
	{
		pos = list_nth(&io->datas_out, 1);
		data = CONTAINER_OF(pos, t_data, list);
		data_free(data);
	}
	close_socket(io->sock);
	io->connected = false;
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

	server = get_server();

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

	server = get_server();
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

	server = get_server();
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

	server = get_server();

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
	int			sock;
	ssize_t		size;

	server = get_server();
	if (!FD_ISSET(server->listen, &server->fds[RFDS]))
		return (true);
	sock = accept_connection(server->listen);
	if (sock == -1)
		return (false);
	if (!create_io(sock))
	{
		size = send(sock, MSG("ERROR Too many users\n"), MSG_DONTWAIT);
		if (size == -1)
			perror("send", errno);
		close_socket(sock);
		return (false);
	}
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

	server = get_server();
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

	server = get_server();
	if (!FD_ISSET(io->sock, &server->fds[WFDS]))
		return (true);
	pos = list_nth(&io->datas_out, 1);
	data = CONTAINER_OF(pos, t_data, list);
	return (int_send_data(io->sock, data));
}

/*
** treat request
*/
int		request_user(int argc, char **argv, t_user *user, t_io *io)
{
	t_data	*data;

	data = NULL;
	if (argc > 2)
		return (E2BIG);
	if (argc == 1)
	{
		if (ft_strlen(user->name) == 0)
			return (ENOTREGISTER);
		data = msg_success(user->name);
		if (!data)
			return (ENOMEM);
		push_data(data, &io->datas_out);
	}
	else if (argc == 2)
	{
		ft_strncpy(user->name, argv[1], NAME_SIZE_MAX);
		data = msg_success(user->name);
		if (!data)
			return (ENOMEM);
		push_data(data, &io->datas_out);
	}
	return (ESUCCESS);
}

int		request_quit(int argc, char **argv, t_user *user, t_io *io)
{
	(void)argv;
	(void)user;
	if (argc > 1)
		return (E2BIG);
	send(io->sock, MSG(SUCCESS "\n"), MSG_DONTWAIT);
	delete_io(io);
	return (ESUCCESS);
}

#include <sys/utsname.h>	/* uname() */
int		request_syst(int argc, char **argv, t_user *user, t_io *io)
{
	t_data			*data;
	char			buf[128];
	struct utsname	ubuf;

	LOG_DEBUG("");
	(void)argv;
	(void)user;
	if (argc > 1)
		return (E2BIG);
	if (uname(&ubuf))
		return (errno);
	ft_strncpy(buf, ubuf.sysname, sizeof(buf));
	ft_strncat(buf, " ", sizeof(buf));
	ft_strncat(buf, ubuf.release, sizeof(buf));
	data = msg_success(buf);
	if (!data)
		return (ENOMEM);
	push_data(&io->datas_out, data);
	return (ESUCCESS);
}

/*
** get request
*/
bool	get_request(t_data *in, t_buf *request)
{
	char	*pt;

	pt = ft_memchr(in->bytes, '\n', in->size);
	if (pt == NULL)
		return (false);
	request->bytes = in->bytes;
	request->size = (size_t)(pt - in->bytes);
	return (true);
}

bool	split_request(t_buf *request, int *argc, char ***argv)
{
	size_t	i;

	request->bytes[request->size] = '\0';
	*argv = strsplit_whitespace(request->bytes);
	request->bytes[request->size] = '\n';
	if (!*argv)
		return (false);
	*argc = 0;
	while ((*argv)[*argc])
		(*argc) += 1;
	if (*argc <= 0)
		return (false);
	i = 0;
	while ((*argv)[0][i])
	{
		(*argv)[0][i] = (char)ft_toupper((*argv)[0][i]);
		i++;
	}
	return (true);
}

typedef struct	s_request
{
	char	*str;
	int		(*func)(int, char **, t_user *, t_io *);
}				t_request;

int		match_request(int argc, char **argv, t_user *user, t_io *io)
{
	static t_request	requests[] = {
		//{ "PWD",  request_pwd },
		{ "USER", request_user },
		{ "QUIT", request_quit },
		{ "SYST", request_syst },
		//{ "LS",   request_ls },
		//{ "CD",   request_cd },
		//{ "GET",  request_get },
		//{ "PUT",  request_put }
	};
	size_t				i;

	i = 0;
	while (i < sizeof(requests) / sizeof(requests[0]))
	{
		if (!ft_strcmp(argv[0], requests[i].str))
			return (requests[i].func(argc, argv, user, io));
		i++;
	}
	return (ENOTSUP);
}

bool	treat_request(int ac, char **av, t_user *user, t_io *io)
{
	t_data	*data;
	char	*err;
	int		errnum;

	errnum = match_request(ac, av, user, io);
	if (errnum)
	{
		err = strerror(errnum);
		if (!err)
			err = "Undefined error";
		data = msg_error(err);
		if (!data)
			return (false);
	}
	return (true);
}

bool	treat_input_data(t_io *io)
{
	int			argc;
	char		**argv;
	t_user		*user;
	t_buf		request;
	bool		status;

	LOG_DEBUG("data_in {%.*s}", (int)io->data_in.size, io->data_in.bytes);
	if (!get_request(&io->data_in, &request))
		return (true);
	LOG_DEBUG("request {%.*s}", (int)request.size, request.bytes);
	if (!split_request(&request, &argc, &argv))
		return (false);
	ft_memmove(io->data_in.bytes, io->data_in.bytes + request.size, io->data_in.size - request.size);
	io->data_in.size -= request.size;
	LOG_DEBUG("data_in {%.*s}", (int)io->data_in.size, io->data_in.bytes);
	user = get_user(io->sock);
	LOG_DEBUG("now treating request ..");
	status = treat_request(argc, argv, user, io);
	argc = 0;
	while (argv[argc])
		free(argv[argc++]);
	free(argv);
	LOG_DEBUG("Ok ..");
	return (status);
}

/*
** server loop
*/
bool	foreach_io(bool (*io_func)(t_io *))
{
	t_server	*server;
	t_io		*io;
	t_list		*pos;
	int			nfails;

	server = get_server();
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

	if (!foreach_io(&recv_data))
		LOG_ERROR("foreach_io failed on recv_data");
	if (!foreach_io(&send_data))
		LOG_ERROR("foreach_io failed on send_data");

	// check requests
	if (!foreach_io(&treat_input_data))
		LOG_ERROR("foreach_io failed on treat_input_data");

	return (true);
}

/*
** main
*/
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
