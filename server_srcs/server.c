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
	if (!fifo_create(MSG_SIZE_MAX + 1, USER_COUNT_MAX, &ftp->fifo.datas))
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
	user->nextfds = WFDS;
	set_move(user, ftp->sets);
	return (true);
}

bool	request_user(t_ftp *ftp, t_user *user, int argc, char **argv)
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

bool	request_quit(t_ftp *ftp, t_user *user, int argc, char **argv)
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
bool	request_syst(t_ftp *ftp, t_user *user, int argc, char **argv)
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

bool	request_pwd(t_ftp *ftp, t_user *user, int argc, char **argv)
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
bool	request_ls(t_ftp *ftp, t_user *user, int argc, char **argv)
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

bool	request_cd(t_ftp *ftp, t_user *user, int argc, char **argv)
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
bool	open_file_for_reading(char *file, t_ftp *ftp, t_user *user)
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
		data_errno(errno, &user->data);
		return (false);
	}
	if (fstat(fd, &sb))
	{
		data_errno(errno, &user->data);
		close(fd);
		return (false);
	}
	if (!S_ISREG(sb.st_mode))
	{
		close(fd);
		data_update(MSG("Not a regular file"), &user->data);
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
		data_errno(errno, &user->data);
		close(fd);
		return (false);
	}

	fifo_store(user->data.bytes, &ftp->fifo.datas);

	user->data.fd = fd;
	user->data.bytes = map;
	user->data.offset = 0;
	user->data.size = (size_t)sb.st_size;

	return (true);
}

bool	open_file_for_writing(char *file, size_t size, t_ftp *ftp, t_user *user)
{
	char		path[PATH_SIZE_MAX + 1];
	int			fd;
	char		*map;

	get_path(user->pwd, file, PATH_SIZE_MAX, path);
	LOG_DEBUG("open {%s} for writing", path);

	fd = open(path, O_CREAT | O_TRUNC | O_RDWR, 0777);
	if (fd == -1)
	{
		data_errno(errno,&user->data);
		return (false);
	}

	if (ftruncate(fd, (off_t)size))
	{
		data_errno(errno, &user->data);
		close(fd);
		return (false);
	}

	map = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (map == MAP_FAILED)
	{
		data_errno(errno, &user->data);
		close(fd);
		return (false);
	}

	fifo_store(user->data.bytes, &ftp->fifo.datas);

	user->data.fd = fd;
	user->data.bytes = map;
	user->data.offset = 0;
	user->data.size = size;

	return (true);
}

void	close_file(t_ftp *ftp, t_user *user)
{
	LOG_DEBUG("close file");

	if (munmap(user->data.bytes, user->data.size) == -1)
		LOG_ERROR("munmap: %s", strerror(errno));

	user->data.bytes = fifo_pull(&ftp->fifo.datas);
	data_teardown(&user->data);

	close(user->data.fd);
	user->data.fd = -1;

	data_success(&user->data);
	user->nextfds = WFDS;
}

# define FILE_SIZE_MAX	64
bool	request_get(t_ftp *ftp, t_user *user, int argc, char **argv)
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

bool	request_put(t_ftp *ftp, t_user *user, int argc, char **argv)
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
	bool	(*func)(t_ftp *, t_user *, int, char **);
	t_fds	fds;
}				t_request;

bool	request(t_ftp *ftp, t_user *user, int argc, char **argv)
{
	static t_request	requests[] = {
		{ "PWD",  request_pwd,  WFDS },
		{ "USER", request_user, WFDS },
		{ "QUIT", request_quit, NOFDS },
		{ "SYST", request_syst, WFDS },
		{ "LS",   request_ls,   WFDS },
		{ "CD",   request_cd,   WFDS },
		{ "GET",  request_get,  WFDS },
		{ "PUT",  request_put,  RFDS }
	};
	size_t				i;

	data_teardown(&user->data);
	i = 0;
	while (i < sizeof(requests) / sizeof(requests[0]))
	{
		if (!strcmp(argv[0], requests[i].str))
		{
			user->nextfds = requests[i].fds;
			return (requests[i].func(ftp, user, argc, argv));
		}
		i++;
	}
	data_update(MSG("Invalid request"), &user->data);
	return (false);
}

bool	request_split(char *data, char ***argv, int *argc)
{
	size_t	i;

	*argv = strsplit_whitespace(data);
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

bool	request_init(t_ftp *ftp, t_user *user, char *data)
{
	int		argc;
	char	**argv;
	bool	status;
	int		i;

	if (!request_split(data, &argv, &argc))
		return (false);
	status = request(ftp, user, argc, argv);
	if (!status)
	{
		user->nextfds = WFDS;
		data_error(&user->data);
	}
	else if (user->used == true && user->data.fd == -1)
	{
		data_success(&user->data);
	}
	i = 0;
	while (i < argc)
		free(argv[i++]);
	free(argv);
	return (status);
}

bool	rfds(t_ftp *ftp, t_user *user)
{
	char		data[MSG_SIZE_MAX + 1];
	ssize_t		size;
	size_t		nb_write;

	EASY_DEBUG_IN;
	user->nextfds = RFDS;
	if (user->data.size)
	{
		ft_memcpy(data, user->data.bytes, user->data.size);
		data_teardown(&user->data);
	}
	size = recv(user->sock, data + user->data.size, MSG_SIZE_MAX - user->data.size, MSG_DONTWAIT);
	if (size == 0)
	{
		user->used = false;
		return (true);
	}
	if (size < 0)
	{
		LOG_ERROR("recv: %s", strerror(errno));
		if (user->data.fd != -1)
		{
			close_file(ftp, user);
		}
		return (false);
	}
	size += user->data.size;

	LOG_DEBUG("recv {%.*s}", (int)size, data);

	if (user->data.fd == -1)
	{
		char	*pt;
		size_t	index;

		pt = ft_memchr(data, '\n', (size_t)size);
		if (pt == NULL)
		{
			data_update(data, (size_t)size, &user->data);
			return (true);
		}
		*pt = '\0';

		index = (size_t)(pt - data);

		request_init(ftp, user, data);

		size -= index + 1;
		if (size < 0)
		{
			LOG_ERROR("size %ld", size);
			return (false);
		}
		ft_memmove(data, data + (index + 1), (size_t)size);
	}
	if (user->data.fd != -1)
	{
		nb_write = (user->data.offset + (size_t)size > user->data.size) ?
			user->data.size - user->data.offset : (size_t)size;

		LOG_DEBUG("write {%.*s} at offset {%zu} size max {%zu}", (int)nb_write, data, user->data.offset, user->data.size);
		ft_memcpy(user->data.bytes + user->data.offset, data, nb_write);
		user->data.offset += nb_write;

		if (user->data.offset == user->data.size)
		{
			close_file(ftp, user);
		}
	}

	EASY_DEBUG_OUT;
	return (true);
}

bool	wfds(t_ftp *ftp, t_user *user)
{
	char	*data;
	size_t	data_size;
	ssize_t	size;

	EASY_DEBUG_IN;
	user->nextfds = NOFDS;

	LOG_DEBUG("send data offset %zu size %zu", user->data.offset, user->data.size);

	data = user->data.bytes + user->data.offset;
	data_size = user->data.size - user->data.offset;

	size = send(user->sock, data, data_size, MSG_DONTWAIT);
	if (size < 0)
	{
		LOG_ERROR("send: %s", strerror(errno));
		if (user->data.fd != -1)
			close_file(ftp, user);
		return (false);
	}

	LOG_DEBUG("send {%.*s}", (int)size, data);

	user->data.offset += (size_t)size;
	if (user->data.offset == user->data.size)
	{
		if (user->data.fd != -1)
		{
			close_file(ftp, user);
		}
		else
		{
			data_teardown(&user->data);
			user->nextfds = RFDS;
		}
	}

	EASY_DEBUG_OUT;
	return (true);
}

void	check_fd_isset(t_ftp *ftp, t_fds fds, bool (*io)(t_ftp *, t_user *), t_list *users)
{
	t_user	*user;
	t_list	*pos;

	while (!list_is_empty(&ftp->sets[fds].users))
	{
		pos = list_nth(&ftp->sets[fds].users, 1);
		user = CONTAINER_OF(pos, t_user, set);
		if (FD_ISSET(user->sock, &ftp->sets[fds].fds))
		{
			if (!io(ftp, user))
			{
				user_del(&ftp->fifo, user);
				continue ;
			}
		}
		else
		{
			user->nextfds = user->fds;
		}
		if (user->used == false)
			user_del(&ftp->fifo, user);
		else
			list_move(&user->set, users);
	}
}

void	check_fds_isset(t_ftp *ftp)
{
	t_list	users;
	t_user	*user;
	t_list	*pos;

	INIT_LIST_HEAD(&users);

	check_fd_isset(ftp, RFDS, &rfds, &users);
	check_fd_isset(ftp, WFDS, &wfds, &users);

	while (!list_is_empty(&users))
	{
		pos = list_nth(&users, 1);
		user = CONTAINER_OF(pos, t_user, set);
		if (user->fds == NOFDS)
		{
			// Close sockets on error
			LOG_ERROR("user in NOFDS");
			user_show(user);
		}
		else
		{
			set_move(user, ftp->sets);
		}
	}
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

		check_fds_isset(ftp);

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
