#include "server.h"
#include "log.h"
#include "libft.h"
#include "strerror.h"
#include "printf.h"

#include <sys/socket.h>
#include <stdlib.h>

/*
** user
*/
t_user	*get_user(int sock, t_server *server)
{
	if (sock < 0 || (unsigned long)sock >= sizeof(server->user_array) / sizeof(server->user_array[0]))
		return (NULL);
	return (&server->user_array[sock]);
}

/* //TEMP */
/*
** data file
*/
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

/*
** recv file
*/
int			data_recv_file(char *file, size_t size, t_data *data)
{
	int		fd;
	char	*map;
	int		err;

	LOG_DEBUG("recv_file {%s} size %zu", file, size);
	fd = open(file, O_CREAT | O_EXCL | O_RDWR, 0644);
	if (fd == -1)
		return (errno);
	if (size == 0)
	{
		close(fd);
		return (ESUCCESS);
	}

	if (ftruncate(fd, size))
	{
		err = errno;
		close(fd);
		return (err);
	}

	map = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (map == MAP_FAILED)
	{
		err = errno;
		close(fd);
		return (err);
	}
	data->fd = fd;
	data->bytes = map;
	data->offset = 0;
	data->size = 0;
	data->size_max = size;
	return (ESUCCESS);
}

/*
** send file
*/
int		get_file_size(int fd, size_t *size)
{
	struct stat	sb;

	if (fstat(fd, &sb))
		return (errno);
	if (!S_ISREG(sb.st_mode))
		return (EFTYPE);
	*size = (size_t)sb.st_size;
	return (ESUCCESS);
}

int		data_send_file(char *file, t_data *data)
{
	int		fd;
	char	*map;
	size_t	size;
	int		err;

	fd = open(file, O_RDONLY);
	if (fd == -1)
		return (errno);
	err = get_file_size(fd, &size);
	if (err)
	{
		close(fd);
		return (err);
	}
	if (size == 0)
	{
		close(fd);
		return (ESUCCESS);
	}

	map = mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (map == MAP_FAILED)
	{
		err = errno;
		close(fd);
		return (err);
	}

	data->fd = fd;
	data->bytes = map;
	data->offset = 0;
	data->size = size;
	data->size_max = size;

	return (ESUCCESS);
}

/*
** data free
*/
void		free_file(t_data *data)
{
	if (munmap(data->bytes, data->size_max))
		perror("munmap", errno);
	if (close(data->fd))
		perror("close", errno);
}

void		free_data(t_data *data)
{
	list_del(&data->list);
	if (data->fd != -1)
		free_file(data);
	free(data);
}

/*
** user
*/
void		user_init(t_user *user, char *home)
{
	user->name[0] = '\0';
	ft_strncpy(user->home, home, PATH_SIZE_MAX);
	ft_strncpy(user->pwd, home, PATH_SIZE_MAX);
}

/*
** io
*/
t_io		*get_io(int sock, t_server *server)
{
	if (sock < 0 || (unsigned long)sock >= sizeof(server->io_array) / sizeof(server->io_array[0]))
		return (NULL);
	return (&server->io_array[sock]);
}

void		io_input_teardown(t_io *io)
{
	io->data_in.fd = -1;
	io->data_in.bytes = io->input_buffer;
	io->data_in.size_max = INPUT_BUFFER_SIZE;
	io->data_in.size = 0;
	io->data_in.offset = 0;
}

#include "sock.h"
int			create_io(int sock, t_server *server)
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
	return (ESUCCESS);
}

void		delete_io(t_io *io)
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

void		sets_prepare(t_server *server, int *nfds)
{
	FD_ZERO(&server->fds[RFDS]);
	FD_ZERO(&server->fds[WFDS]);

	FD_SET(server->listen, &server->fds[RFDS]);
	*nfds = server->listen + 1;

	fds_set(&server->io_list, server->fds, nfds);
}

/*
** server open
*/
#include "listen_socket.h"
#include "sock.h"
bool	server_open(char *host, int port, t_server *server)
{
	t_io		*io;
	size_t		i;

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

	char	*home = getenv("HOME");
	if (!home)
		home = "/tmp";
	ft_strncpy(server->home, home, PATH_SIZE_MAX);
	LOG_DEBUG("home: %s", server->home);

	return (true);
}

/*
** server close
*/
bool	server_close(t_server *server)
{
	close_socket(server->listen);
	return (true);
}

/*
** server loop
*/
bool		fds_availables(int nfds, t_server *server, int *ready)
{
	struct timeval	tv;

	tv.tv_sec = 10;
	tv.tv_usec = 0;

	*ready = select(
		nfds,
		&server->fds[RFDS],
		&server->fds[WFDS],
		NULL,
		&tv
	);
	if (*ready == -1)
	{
		if (errno != EINTR)
			return (false);
	}

	return (true);
}

bool	handle_new_connections(t_server *server, bool *new_user)
{
	int			sock;
	ssize_t		size;
	int			errnum;
	char		*err;

	*new_user = false;
	if (!FD_ISSET(server->listen, &server->fds[RFDS]))
		return (true);
	sock = accept_connection(server->listen);
	if (sock == -1)
		return (false);
	errnum = create_io(sock, server);
	if (errnum)
	{
		err = strerror(errnum);
		size = send(sock, err, ft_strlen(err), MSG_DONTWAIT);
		if (size == -1)
			perror("send", errno);
		close_socket(sock);
		return (false);
	}
	LOG_DEBUG("new user connected");
	*new_user = true;
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
	if (data->nbytes == 0)
		return (false);
	data->size += (size_t)data->nbytes;
	return (true);
}

bool	recv_data(t_server *server, t_io *io)
{
	if (!FD_ISSET(io->sock, &server->fds[RFDS]))
		return (true);
	if (!int_recv_data(io->sock, &io->data_in))
	{
		delete_io(io);
		return (false);
	}
	if (io->data_in.size == io->data_in.size_max)
	{
		if (io->data_in.fd != -1)
			free_file(&io->data_in);
		io_input_teardown(io);
	}
	return (true);
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
	data->offset += (size_t)data->nbytes;
	return (true);
}

bool	send_data(t_server *server, t_io *io)
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

/*
** treat request
*/
int	request_pwd(int argc, char **argv, t_user *user, t_io *io)
{
	t_data	*data;

	(void)argv;
	if (argc > 1)
		return (E2BIG);
	data = msg_success(user->pwd);
	if (!data)
		return (errno);
	list_add_tail(&data->list, &io->datas_out);
	return (ESUCCESS);
}

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
			return (errno);
		list_add_tail(&data->list, &io->datas_out);
	}
	else if (argc == 2)
	{
		ft_strncpy(user->name, argv[1], NAME_SIZE_MAX);
		data = msg_success(user->name);
		if (!data)
			return (errno);
		list_add_tail(&data->list, &io->datas_out);
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
		return (errno);
	list_add_tail(&data->list, &io->datas_out);
	return (ESUCCESS);
}

#include <dirent.h>
int		request_ls(int argc, char **argv, t_user *user, t_io *io)
{
	DIR				*dp;
	struct dirent	*ep;
	char			path[PATH_SIZE_MAX + 1];

	if (argc > 2)
		return (E2BIG);

	if (argv[1] == NULL)
		ft_strncpy(path, user->pwd, PATH_SIZE_MAX);
	else
		concat_path(user->pwd, argv[1], path);

	int	err;
	err = move_directory(user, path, PATH_SIZE_MAX);
	if (err)
		return (err);

	if ((dp = opendir(path)) == NULL)
		return (errno);

	char			buf[4096];
	off_t			offset;

	offset = 0;
	while ((ep = readdir(dp)) != NULL)
	{
		if (ep->d_name[0] == '.')
			continue ;
		offset = concat_safe(buf, offset, sizeof(buf) - 1, ep->d_name);
		offset = concat_safe(buf, offset, sizeof(buf) - 1, " ");
	}
	if (offset > 0 && buf[offset - 1] != '$')
		offset -= 1;

	buf[offset] = '\0';

	closedir(dp);

	t_data		*data;

	data = msg_success(buf);
	if (!data)
		return (errno);
	list_add_tail(&data->list, &io->datas_out);

	return (ESUCCESS);
}

int		request_cd(int argc, char **argv, t_user *user, t_io *io)
{
	char	path[PATH_SIZE_MAX + 1];
	char	*pwd_argv[2];
	int		err;

	if (argc > 2)
		return (E2BIG);

	if (argc == 1)
		ft_strncpy(path, user->home, PATH_SIZE_MAX);
	else
		concat_path(user->pwd, argv[1], path);
	if ((err = move_directory(user, path, PATH_SIZE_MAX)))
		return (err);
	ft_strncpy(user->pwd, path, PATH_SIZE_MAX);
	pwd_argv[0] = "PWD";
	pwd_argv[1] = NULL;
	return (request_pwd(1, pwd_argv, user, io));
}

int		request_get(int argc, char **argv, t_user *user, t_io *io)
{
	t_data		*file;
	t_data		*file_size;
	char		*file_size_str;
	char		path[PATH_SIZE_MAX + 1];
	int			err;

	if (argc != 2)
		return (EARGS);
	if (ft_strchr(argv[1], '/'))
		return (EINVAL);
	concat_path(user->pwd, argv[1], path);
	file = alloc_data();
	if (!file)
		return (errno);
	err = data_send_file(path, file);
	if (err)
	{
		free_data(file);
		return (err);
	}
	file_size_str = ft_itoa(file->size_max);
	if (!file_size_str)
	{
		err = errno;
		free_data(file);
		return (err);
	}
	file_size = msg_success(file_size_str);
	if (!file_size)
	{
		err = errno;
		free(file_size_str);
		free_data(file);
		return (err);
	}
	free(file_size_str);
	list_add_tail(&file_size->list, &io->datas_out);
	list_add_tail(&file->list, &io->datas_out);
	return (ESUCCESS);
}

int		request_put(int argc, char **argv, t_user *user, t_io *io)
{
	t_data	*data;
	char	path[PATH_SIZE_MAX + 1];
	int		size;
	int		err;

	if (argc != 3)
		return (EARGS);
	size = ft_atoi(argv[2]);
	if (size < 0)
		return (EINVAL);
	if (ft_strchr(argv[1], '/'))
		return (EINVAL);
	concat_path(user->pwd, argv[1], path);
	err = data_recv_file(path, size, &io->data_in);
	if (err)
		return (err);
	data = msg_success("");
	if (!data)
		return (errno);
	list_add_tail(&data->list, &io->datas_out);
	return (0);
}

typedef struct	s_request
{
	char	*str;
	int		(*func)(int, char **, t_user *, t_io *);
}				t_request;

int		match_request(int argc, char **argv, t_user *user, t_io *io)
{
	static t_request	requests[] = {
		{ "PWD",  request_pwd },
		{ "USER", request_user },
		{ "QUIT", request_quit },
		{ "SYST", request_syst },
		{ "LS",   request_ls },
		{ "CD",   request_cd },
		{ "GET",  request_get },
		{ "PUT",  request_put }
	};
	size_t				i;

	i = 0;
	while (i < sizeof(requests) / sizeof(requests[0]))
	{
		if (!ft_strcmp(argv[0], requests[i].str))
			return (requests[i].func(argc, argv, user, io));
		i++;
	}
	return (EBADRQC);
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
		list_add_tail(&data->list, &io->datas_out);
	}
	LOG_DEBUG("user {%s} request treated", user->name);
	return (true);
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
	i = 0;
	while ((*argv)[0][i])
	{
		(*argv)[0][i] = (char)ft_toupper((*argv)[0][i]);
		i++;
	}
	return (true);
}

bool	treat_input_data(t_server *server, t_io *io)
{
	int			argc;
	char		**argv;
	t_user		*user;
	t_buf		request;
	bool		status;

	if (io->data_in.fd != -1)
		return (true);
	if (!get_request(&io->data_in, &request))
		return (true);
	split_request(&request, &argc, &argv);
	request.size += 1;
	ft_memmove(io->data_in.bytes, io->data_in.bytes + request.size, io->data_in.size - request.size);
	io->data_in.size -= request.size;
	if (!argv)
		return (false);
	user = get_user(io->sock, server);
	status = treat_request(argc, argv, user, io);
	argc = 0;
	while (argv[argc])
		free(argv[argc++]);
	free(argv);
	return (status);
}

/*
** server loop
*/
void	foreach_io(t_server *server, bool (*io_func)(t_server *, t_io *))
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

bool	server_loop(t_server *server)
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
	foreach_io(server, &recv_data);

	foreach_io(server, &treat_input_data);

	return (true);
}

/*
** main
*/
#include <unistd.h>
int		main(int argc, char **argv)
{
	t_server	server;
	char		*host;
	int			port;
	int			exit_status;

	if (argc != 3)
	{
		ft_fprintf(2, "Usage: %s host port\n", argv[0]);
		return (EXIT_FAILURE);
	}
	host = argv[1];
	port = ft_atoi(argv[2]);
	if (!server_open(host, port, &server))
	{
		LOG_ERROR("server_open failed host {%s} port {%s}", argv[1], argv[2]);
		return (EXIT_FAILURE);
	}
	exit_status = EXIT_SUCCESS;
	while (1)
	{
		if (!server_loop(&server))
		{
			exit_status = EXIT_FAILURE;
			break ;
		}
	}
	if (!server_close(&server))
	{
		LOG_ERROR("server_close failed");
		return (EXIT_FAILURE);
	}
	return (exit_status);
}
