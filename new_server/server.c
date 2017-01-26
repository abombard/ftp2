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

size_t	concat_safe(char *buf, size_t offset, size_t size_max, char *s)
{
	size_t	size;

	if (s == NULL)
		LOG_ERROR("s %p", s);
	size = ft_strlen(s);
	if (offset + size > size_max)
	{
		size = size_max - offset - 1;
		buf[size_max - 1] = '$';
	}
	ft_memcpy(buf + offset, s, size);
	return (offset + size);
}

void	get_path(char *pwd, char *in_path, size_t size_max, char *path)
{
	if (in_path[0] == '/')
	{
		ft_strncpy(path, in_path, size_max);
	}
	else
	{
		ft_strncpy(path, pwd, size_max);
		ft_strncat(path, "/", size_max);
		ft_strncat(path, in_path, size_max);
	}
}

int		check_perm(t_user *user, char *path)
{
	struct stat	sb;
	char		c;
	char		*pt;

	if (lstat(path, &sb))
		return (errno);
	if (S_ISDIR(sb.st_mode))
		
}

/*
** user
*/
t_user	*get_user(int sock, t_server *server)
{
	if (sock < 0 || (unsigned long)sock >= sizeof(server->user_array) / sizeof(server->user_array[0]))
		return (NULL);
	return (&server->user_array[sock]);
}

/*
** datas
*/
void		data_clear(t_data *data)
{
	INIT_LIST_HEAD(&data->list);
	data->fd = -1;
	data->bytes = NULL;
	data->offset = 0;
	data->size = 0;
	data->size_max = 0;
}

/*
** data msg
*/
#include <stdlib.h>
t_data		*alloc_data(void)
{
	t_data	*data;

	data = (t_data *)malloc(sizeof(t_data));
	if (!data)
	{
		perror("malloc", errno);
		return (NULL);
	}
	data_clear(data);
	return (data);
}

t_data		*alloc_data_msg(size_t size)
{
	t_data	*data;

	data = malloc(sizeof(t_data) + size + 1);
	if (!data)
	{
		perror("malloc", errno);
		return (NULL);
	}
	data_clear(data);
	data->bytes = (void *)data + sizeof(t_data);
	data->size_max = size;
	return (data);
}

t_data		*msg_success(char *msg)
{
	t_data	*data;
	size_t	msg_size;

	msg_size = SUCCESS_SIZE + sizeof(" ") - 1 + ft_strlen(msg);
	data = alloc_data_msg(msg_size);
	if (!data)
		return (NULL);
	ft_strncpy(data->bytes, SUCCESS, data->size_max);
	ft_strncat(data->bytes, " ", data->size_max);
	ft_strncat(data->bytes, msg, data->size_max);
	data->size = ft_strlen(data->bytes);
	data->bytes[data->size] = '\n';
	data->size += 1;
	LOG_DEBUG("msg {%.*s}", (int)data->size, data->bytes);
	return (data);
}

t_data		*msg_error(char *msg)
{
	t_data	*data;
	size_t	msg_size;

	msg_size = ERROR_SIZE + sizeof(" ") - 1 + ft_strlen(msg);
	data = alloc_data_msg(msg_size);
	if (!data)
		return (NULL);
	ft_strncpy(data->bytes, ERROR, data->size_max);
	ft_strncat(data->bytes, " ", data->size_max);
	ft_strncat(data->bytes, msg, data->size_max);
	data->size = ft_strlen(data->bytes);
	data->bytes[data->size] = '\n';
	data->size += 1;
	LOG_DEBUG("msg {%.*s}", (int)data->size, data->bytes);
	return (data);
}

void		push_data(t_data *data, t_list *data_list)
{
	list_add_tail(&data->list, data_list);
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
		return (err);
	if (size == 0)
	{
		// What if file is empty ?
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
	LOG_DEBUG("free {%.*s} fd %d size %zu size_max %zu", (int)data->size, data->bytes, data->fd, data->size, data->size_max);
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
	data_clear(&io->data_in);
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
		free_data(&io->data_in);
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
		FD_SET(io->sock, &fds[EFDS]);
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
	FD_ZERO(&server->fds[EFDS]);

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
bool	fds_availables(int nfds, t_server *server, int *ready)
{
	struct timeval	tv;

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
		{
			LOG_DEBUG("select caught a signal");
			return (true);
		}
		perror("select", errno);
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
	{
		//TEMP EOF
		LOG_DEBUG("reached EOF");
		return (false);
	}
	LOG_DEBUG("recv {%.*s}", (int)data->nbytes, data->bytes + data->size);
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
		{
			free_data(&io->data_in);
			io_input_teardown(io);
		}
	}
	return (true);
}

bool	check_efds(t_server *server, t_io *io)
{
	if (!FD_ISSET(io->sock, &server->fds[EFDS]))
		return (true);
	LOG_DEBUG("sock %d in EFDS", io->sock);
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
	LOG_DEBUG("send {%.*s}", (int)data->nbytes, data->bytes + data->offset);
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
	push_data(data, &io->datas_out);
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
		push_data(data, &io->datas_out);
	}
	else if (argc == 2)
	{
		ft_strncpy(user->name, argv[1], NAME_SIZE_MAX);
		data = msg_success(user->name);
		if (!data)
			return (errno);
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
	push_data(data, &io->datas_out);
	return (ESUCCESS);
}

#include <dirent.h>
int		request_ls(int argc, char **argv, t_user *user, t_io *io)
{
	DIR				*dp;
	struct dirent	*ep;
	char			path[PATH_SIZE_MAX + 1];

	char			buf[4096];
	off_t			offset;

	if (argc > 2)
		return (E2BIG);

	if (argv[1] == NULL)
		ft_strncpy(path, user->pwd, PATH_SIZE_MAX);
	else
		get_path(user->pwd, argv[1], PATH_SIZE_MAX, path);

	offset = 0;

	LOG_DEBUG("path {%s}", path);

	if ((dp = opendir(path)) == NULL)
		return (errno);

	while ((ep = readdir(dp)) != NULL)
	{
		if (ep->d_name[0] == '.')
			continue ;
		offset = concat_safe(buf, offset, sizeof(buf), ep->d_name);
		offset = concat_safe(buf, offset, sizeof(buf), " ");
	}
	if (offset > 0)
		offset -= 1;

	buf[offset] = '\0';

	closedir(dp);

	t_data		*data;

	data = msg_success(buf);
	if (!data)
		return (errno);
	push_data(data, &io->datas_out);

	return (ESUCCESS);
}

int		request_cd(int argc, char **argv, t_user *user, t_io *io)
{
	char	*pwd_argv[2];
	char	path[PATH_SIZE_MAX + 1];
	char	pwd[PATH_SIZE_MAX + 1];
	size_t	size;

	if (argc > 2)
		return (E2BIG);

	if (argc == 1)
		ft_strncpy(path, user->home, PATH_SIZE_MAX);
	else
		get_path(user->pwd, argv[1], PATH_SIZE_MAX, path);

	if (chdir(path))
		return (errno);

	if (!getcwd(pwd, PATH_SIZE_MAX))
		return (errno);

	size = ft_strlen(user->home);
	if (ft_memcmp(pwd, user->home, size) ||
		(pwd[size] != '/' && pwd[size] != '\0'))
		return (EPERM);

	ft_strncpy(user->pwd, pwd, PATH_SIZE_MAX);

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
	file = alloc_data();
	if (!file)
		return (errno);
	get_path(user->pwd, argv[1], PATH_SIZE_MAX, path);
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
	push_data(file_size, &io->datas_out);
	push_data(file, &io->datas_out);
	return (ESUCCESS);
}

int		request_put(int argc, char **argv, t_user *user, t_io *io)
{
	char	path[PATH_SIZE_MAX + 1];
	int		size;
	int		err;

	(void)user;
	if (argc != 3)
		return (EARGS);
	size = ft_atoi(argv[2]);
	if (size < 0)
		return (EINVAL);
	get_path(user->pwd, argv[1], PATH_SIZE_MAX, path);
	err = data_recv_file(path, size, &io->data_in);
	if (err)
		return (err);
	return (ESUCCESS);
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
		push_data(data, &io->datas_out);
	}
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
	LOG_DEBUG("data_in {%.*s}", (int)io->data_in.size, io->data_in.bytes);
	if (!get_request(&io->data_in, &request))
		return (true);
	LOG_DEBUG("request {%.*s}", (int)request.size, request.bytes);
	split_request(&request, &argc, &argv);
	request.size += 1;
	ft_memmove(io->data_in.bytes, io->data_in.bytes + request.size, io->data_in.size - request.size);
	io->data_in.size -= request.size;
	LOG_DEBUG("data_in {%.*s}", (int)io->data_in.size, io->data_in.bytes);
	if (!argv)
		return (false);
	user = get_user(io->sock, server);
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
	LOG_DEBUG("foreach_io nfails %d", nfails);
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

	foreach_io(server, &check_efds);

	foreach_io(server, &send_data);
	foreach_io(server, &recv_data);

	foreach_io(server, &treat_input_data);

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
	LOG_DEBUG("catch SIGINT");
	run = false;
}

int		main(int argc, char **argv)
{
	t_server	server;
	char		*host;
	int			port;

	if (argc != 3)
	{
		ft_fprintf(2, "Usage: %s host port\n", argv[0]);
		return (EXIT_FAILURE);
	}
	host = argv[1];
	port = ft_atoi(argv[2]);
	if (signal(SIGINT, sighandler) == SIG_ERR)
	{
		perror("signal", errno);
		return (EXIT_FAILURE);
	}
	if (!server_open(host, port, &server))
	{
		LOG_ERROR("server_open failed host {%s} port {%s}", argv[1], argv[2]);
		return (EXIT_FAILURE);
	}
	while (run)
	{
		if (!server_loop(&server))
		{
			LOG_ERROR("server_loop failed");
			break ;
		}
	}
	if (!server_close(&server))
	{
		LOG_ERROR("server_close failed");
		return (EXIT_FAILURE);
	}
	return (run == false ? EXIT_SUCCESS : EXIT_FAILURE);
}
