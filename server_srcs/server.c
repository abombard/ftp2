#include "server.h"
#include "libft.h"
#include "strerror.h"

#include <string.h>

static volatile bool	run = false;

#include <sys/stat.h>
bool	ftp_open(const char *host, const int port, t_ftp *ftp)
{
	char	*home;

	if (!fifo_create(sizeof(t_user), USER_COUNT_MAX, &ftp->fifo.users))
		LOG_FATAL("fifo_create failed on users");
	if (!fifo_create(MSG_SIZE_MAX + 1, USER_COUNT_MAX * 2, &ftp->fifo.datas))
		LOG_FATAL("fifo_create failed on datas");
	sets_teardown(ftp->sets);
	ftp->home[0] = '\0';
	ftp->sock_listen = listen_socket(host, port);
	if (ftp->sock_listen == -1)
		LOG_FATAL("listen_socket failed");
	home = getenv("HOME");
	if (home == NULL)
		home = "/tmp";
	ft_strncpy(ftp->home, home, PATH_SIZE_MAX);
	LOG_DEBUG("fifo->users size %zu", list_size(&ftp->fifo.users));
	LOG_DEBUG("fifo->datas size %zu", list_size(&ftp->fifo.datas));
	return (true);
}

bool	ftp_close(t_ftp *ftp)
{
	LOG_DEBUG("fifo->users size %zu", list_size(&ftp->fifo.users));
	LOG_DEBUG("fifo->datas size %zu", list_size(&ftp->fifo.datas));
	fifo_destroy(&ftp->fifo.datas);
	fifo_destroy(&ftp->fifo.users);
	socket_close(ftp->sock_listen);
	return (true);
}

int		accept_connection(int sock_listen)
{
	int					sock;
	socklen_t			addr_size;
	struct sockaddr_in	addr;

	addr_size = sizeof (addr);
	sock = accept(sock_listen, (struct sockaddr *)&addr, &addr_size);
	if (sock == -1)
	{
		LOG_ERROR("accept: %s", strerror(errno));
		return (-1);
	}
	if (addr_size > sizeof (addr))
	{
		LOG_ERROR("addr_size %zu sizeof(addr) %zu", (size_t)addr_size, (size_t)sizeof(addr));
		close(sock);
		return (-1);
	}
	return (sock);
}

bool	new_connection(t_ftp *ftp)
{
	t_user	*user;
	int		sock;
	ssize_t	size;

	sock = accept_connection(ftp->sock_listen);
	if (sock == -1)
		return (false);
	user = user_new(&ftp->fifo);
	if (user == NULL)
	{
		size = send(sock, MSG(FTP_ERROR " Too many connections\n"), MSG_DONTWAIT);
		if (size == -1)
			LOG_ERROR("send: %s", strerror(errno));
		socket_close(sock);
		return (false);
	}
	user->sock = sock;
	ft_strncpy(user->home, ftp->home, PATH_SIZE_MAX);
	ft_strncpy(user->pwd, ftp->home, PATH_SIZE_MAX);
	data_success(&user->data);
	fifo_store(&user->data, &ftp->sets[WFDS].datas);
	return (true);
}

bool	request_user(int argc, char **argv, t_ftp *ftp, t_user *user)
{
	(void)ftp;
	if (argc > 2)
	{
		data_errno(EARGS, &user->data);
		return (false);
	}
	if (argc == 1)
	{
		if (strlen(user->name) == 0)
		{
			data_update(MSG("User did not registered"), &user->data);
			return (false);
		}
		data_update(user->name, strlen(user->name), &user->data);
	}
	else if (argc == 2)
	{
		ft_strncpy(user->name, argv[1], USER_NAME_SIZE_MAX);
	}
	return (true);
}

bool	request_quit(int argc, char **argv, t_ftp *ftp, t_user *user)
{
	(void)ftp;
	(void)argv;
	if (argc > 1)
	{
		data_errno(EARGS, &user->data);
		return (false);
	}
	send(user->sock, MSG(FTP_SUCCESS "\n"), MSG_DONTWAIT);
	user->used = false;
	return (true);
}

#include <sys/utsname.h>	/* uname() */
bool	request_syst(int argc, char **argv, t_ftp *ftp, t_user *user)
{
	struct utsname	buf;

	(void)ftp;
	(void)argv;
	if (argc > 1)
	{
		data_errno(EARGS, &user->data);
		return (false);
	}
	if (uname(&buf))
	{
		data_errno(errno, &user->data);
		return (false);
	}
	data_update(buf.sysname, strlen(buf.sysname), &user->data);
	data_update(" ", sizeof(" ") - 1, &user->data);
	data_update(buf.release, strlen(buf.release), &user->data);
	return (true);
}

bool	request_pwd(int argc, char **argv, t_ftp *ftp, t_user *user)
{
	(void)ftp;
	(void)argv;
	if (argc > 1)
	{
		data_errno(EARGS, &user->data);
		return (false);
	}
	data_update(user->pwd, strlen(user->pwd), &user->data);
	return (true);
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

#include <dirent.h>
bool	request_ls(int argc, char **argv, t_ftp *ftp, t_user *user)
{
	DIR				*dp;
	struct dirent	*ep;
	char			path[PATH_SIZE_MAX + 1];

	(void)ftp;
	if (argc > 2)
	{
		data_errno(EARGS, &user->data);
		return (false);
	}

	if (argv[1] == NULL)
		ft_strncpy(path, user->pwd, PATH_SIZE_MAX);
	else
		get_path(user->pwd, argv[1], PATH_SIZE_MAX, path);

	if ((dp = opendir(path)) == NULL)
	{
		data_errno(errno, &user->data);
		return (false);
	}

	while ((ep = readdir(dp)) != NULL)
	{
		if (ep->d_name[0] == '.')
			continue ;
		data_update(ep->d_name, strlen(ep->d_name), &user->data);
		data_update(MSG(" "), &user->data);
	}
	if (user->data.size > 0)
		user->data.size -= sizeof (" ") - 1;

	if (closedir(dp) == -1)
	{
		data_errno(errno, &user->data);
		return (false);
	}

	return (true);
}

bool	request_cd(int argc, char **argv, t_ftp *ftp, t_user *user)
{
	char	*pwd_argv[2];
	char	path[PATH_SIZE_MAX + 1];

	(void)ftp;
	if (argc > 2)
	{
		data_errno(EARGS, &user->data);
		return (false);
	}

	if (argc == 1)
		ft_strncpy(path, user->home, PATH_SIZE_MAX);
	else
		get_path(user->pwd, argv[1], PATH_SIZE_MAX, path);

	if (chdir(path))
	{
		data_errno(errno, &user->data);
		return (false);
	}

	if (!getcwd(user->pwd, PATH_SIZE_MAX))
	{
		data_errno(errno, &user->data);
		return (false);
	}

	pwd_argv[0] = "PWD";
	pwd_argv[1] = NULL;
	return (request_pwd(ftp, user, 1, pwd_argv));
}

# include <fcntl.h>
# include <sys/stat.h>
# include <sys/mman.h>
bool	open_file_for_reading(char *file, t_ftp *ftp, t_user *user, t_data *data)
{
	char		path[PATH_SIZE_MAX + 1];
	int			fd;
	struct stat	sb;
	char		*map;

	get_path(user->pwd, file, PATH_SIZE_MAX, path);
	LOG_DEBUG("open {%s} for reading", path);

	fd = open(path, O_RDONLY);
	if (fd == -1)
	{
		data_errno(errno, data);
		return (false);
	}
	if (fstat(fd, &sb))
	{
		data_errno(errno, data);
		close(fd);
		return (false);
	}
	if (!S_ISREG(sb.st_mode))
	{
		close(fd);
		data_update(MSG("Not a regular file"), data);
		return (false);
	}
	if (sb.st_size == 0)
	{
		close(fd);
		return (true);
	}

	map = mmap(0, (size_t)sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (map == MAP_FAILED)
	{
		data_errno(errno, data);
		close(fd);
		return (false);
	}

	fifo_store(data->bytes, &ftp->fifo.datas);

	data->fd = fd;
	data->bytes = map;
	data->offset = 0;
	data->size = (size_t)sb.st_size;
	data->size_max = (size_t)sb.st_size;

	return (true);
}

bool	open_file_for_writing(char *file, size_t size, t_user *user, t_data *data)
{
	char		path[PATH_SIZE_MAX + 1];
	int			fd;
	char		*map;

	get_path(user->pwd, file, PATH_SIZE_MAX, path);
	LOG_DEBUG("open {%s} for writing", path);

	fd = open(path, O_CREAT | O_TRUNC | O_RDWR, 0777);
	if (fd == -1)
	{
		data_errno(errno, data);
		return (false);
	}

	if (ftruncate(fd, (off_t)size))
	{
		data_errno(errno, data);
		close(fd);
		return (false);
	}

	map = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (map == MAP_FAILED)
	{
		data_errno(errno, data);
		close(fd);
		return (false);
	}

	data->fd = fd;
	data->bytes = map;
	data->offset = 0;
	data->size = 0;
	data->size_max = size;

	return (true);
}

void	close_file(t_data *data)
{
	LOG_DEBUG("close file");
	if (munmap(data->bytes, data->size) == -1)
		LOG_ERROR("munmap: %s", strerror(errno));
	close(data->fd);
	data_teardown(data);
}

# define FILE_SIZE_MAX	64
bool	request_get(int argc, char **argv, t_ftp *ftp, t_user *user)
{
	size_t		size;
	char		size_str[FILE_SIZE_MAX + 1 + (sizeof(FTP_SUCCESS " ") - 1)];
	size_t		size_str_index;
	size_t		size_str_size;

	if (argc != 2)
	{
		data_errno(EARGS, &user->data);
		return (false);
	}
	ASSERT (open_file_for_reading(argv[1], ftp, user));
	size = user->data.size;
	if (size == 0)
	{
		size_str_index = 0;
		size_str_size = sizeof(FTP_SUCCESS " 0\n") - 1;
		ft_memcpy(size_str, FTP_SUCCESS " 0\n", size_str_size);
	}
	else
	{
		size_str_index = FILE_SIZE_MAX;
		size_str[size_str_index--] = '\n';
		size_str_size = 1;
		while (size > 0)
		{
			size_str[size_str_index--] = size % 10 + '0';
			size /= 10;
			size_str_size++;
		}
		size_str_index++;
		size_str_index -= sizeof(FTP_SUCCESS " ") - 1;
		size_str_size += sizeof(FTP_SUCCESS " ") - 1;
		ft_memcpy(size_str + size_str_index,
				FTP_SUCCESS " ", sizeof(FTP_SUCCESS " ") - 1);
	}

	LOG_DEBUG("file size {%.*s}", (int)size_str_size, size_str + size_str_index);

	send(user->sock, size_str + size_str_index, size_str_size, MSG_DONTWAIT);

	return (true);
}

bool	request_put(int argc, char **argv, t_ftp *ftp, t_user *user)
{
	int	size;

	if (argc != 3)
	{
		data_errno(EARGS, &user->data);
		return (false);
	}
	size = atoi(argv[2]);
	if (size < 0)
	{
		data_errno(EINVAL, &user->data);
		return (false);
	}
	ASSERT (open_file_for_writing(argv[1], (size_t)size, ftp, user));
	return (true);
}

typedef struct	s_request
{
	char	*str;
	bool	(*io_data)(int, char **, t_ftp *, t_user *);
}				t_request;

bool	request(int argc, char **argv, t_ftp *ftp, t_data *data)
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

	data_teardown(data);
	i = 0;
	while (i < sizeof(requests) / sizeof(requests[0]))
	{
		if (!strcmp(argv[0], requests[i].str))
			return (requests[i].io_data(argc, argv, ftp, data->user));
		i++;
	}
	data_update(MSG("Invalid request"), data);
	return (false);
}

bool	treat_request(int argc, char **argv, t_ftp *ftp, t_data *data)
{
	bool	status;

	status = request(argc, argv, ftp, data);
	if (!status)
		data_error(data);
	else if (user->used == true && data->fd == -1)
		data_success(data);
	return (status);
}

static bool	request_split(t_buf *request, char ***argv, int *argc)
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

static bool	request_get(t_data *data, t_buf *request)
{
	char	*pt;

	pt = ft_memchr(data->bytes, '\n', data->size);
	if (pt == NULL)
	{
		LOG_DEBUG("data not ready for treatment {%.*s}", (int)data->size, data->bytes);
		return (false);
	}
	request.bytes = data->bytes;
	request.size = (size_t)(pt - data->bytes);
	LOG_DEBUG("request {%.*s}", (int)request.size, request.bytes);
	return (true);
}

bool	treat_data(t_data *data, t_ftp *ftp, bool isset)
{
	t_buf	request;
	int		ac;
	char	**av;
	bool	status;

	(void)isset;
	if (data->fd != -1)
	{
		if (data->offset == data->size)
		{
			close_file(data);
		}
		return (true);
	}
	if (!request_get(data, &request))
		return (true);
	status = request_split(&request, &av, &ac);

	request.size += 1;
	ft_memmove(data->bytes, data->bytes + request.size, data->size - request.size);
	data->size -= request.size;

	if (!status)
		LOG_FATAL("request_split failed");

	status = treat_request(ac, av, data, ftp);
	ac = 0;
	while (av[ac])
		free(av[ac++]);
	free(av);
	return (status);
}

bool	recv_data(t_data *data, t_ftp *ftp, bool isset)
{
	ssize_t	size;

	if (!isset)
		return (true);
	size = recv(data->user->sock, data->bytes + data->size, data->size_max - data->size, MSG_DONTWAIT);
	if (size < 0)
	{
		LOG_ERROR("recv: %s", strerror(errno));
		return (false);
	}
	if (size == 0)
	{
		LOG_DEBUG("recv returned 0");
		return (true);
	}
	LOG_DEBUG("recv {%.*s}", (int)size, data->bytes + data->size);
	data->size += (size_t)size;
	return (true);
}

bool	send_data(t_data *data, t_ftp *ftp, bool isset)
{
	if (!isset)
		return (true);
	size = send(data->user->sock, data->bytes + data->offset, data->size - data->offset, MSG_DONTWAIT);
	if (size < 0)
	{
		LOG_ERROR("send: %s", strerror(errno));
		return (false);
	}
	LOG_DEBUG("send {%.*s}", (int)size, data->bytes + data->offset);
	data->offset += (size_t)size;
	return (true);
}

bool	foreach_data(t_set *set, bool (*io_data)(t_data *, t_ftp *, bool), t_ftp *ftp)
{
	t_data	*data;
	t_list	*pos;
	t_list	*next;
	bool	isset;

	next = &set->datas.next;
	while ((pos = next) && pos != &set->datas && (next = pos->next))
	{
		data = CONTAINER_OF(pos, t_data, fifo);
		isset = FD_ISSET(data->sock, &set->fds);
		ASSERT (io_data(ftp, data, isset));
	}
	return (true);
}

bool	ftp_loop(t_ftp *ftp)
{
	int				ready;
	struct timeval	tv;
	int				nfds;

	run = true;
	while (run)
	{
		nfds = sets_prepare(ftp->sock_listen, ftp->sets);
		LOG_DEBUG("nfds %d", nfds);

		tv.tv_sec = 10;
		tv.tv_usec = 0;

		ready = select(
			nfds,
			&ftp->sets[RFDS].fds,
			&ftp->sets[WFDS].fds,
			&ftp->sets[EFDS].fds,
			&tv
		);
		if (ready == -1)
		{
			if (errno == EINTR)
				continue ;
			LOG_FATAL("select: %s", strerror(errno));
		}
		if (ready == 0)
			continue ;
		LOG_DEBUG("ready %d", ready);

		if (FD_ISSET(ftp->sock_listen, &ftp->sets[RFDS].fds))
		{
			new_connection(ftp);
			ready --;
		}

		ASSERT (foreach_data(&ftp->sets[RFDS], &recv_data, ftp));
		ASSERT (foreach_data(&ftp->sets[RFDS], &treat_data, ftp));
		ASSERT (foreach_data(&ftp->sets[WFDS], &send_data, ftp));
	}

	return (true);
}

void	sighandler(int sig)
{
	(void)sig;
	run = false;
}

int		main(int argc, char **argv)
{
	t_ftp	ftp;
	char	*host;
	int		port;
	int		exit_status;

	signal(SIGINT, sighandler);
	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s host port\n", argv[0]);
		return (1);
	}
	host = argv[1];
	port = ft_atoi(argv[2]);
	LOG_DEBUG("host {%s} port {%d}", host, port);
	if (!ftp_open(host, port, &ftp))
	{
		LOG_ERROR("ftp_open failed host {%s} port {%s}", argv[1], argv[2]);
		return (EXIT_FAILURE);
	}
	exit_status = ftp_loop(&ftp) ? EXIT_SUCCESS : EXIT_FAILURE;
	if (!ftp_close(&ftp))
	{
		LOG_ERROR("ftp_close failed");
		return (EXIT_FAILURE);
	}
	return (exit_status);
}
