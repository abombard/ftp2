#include "ftp.h"
#include "libft.h"
#include <string.h>

static volatile bool	run = false;

bool	fifo_empty(t_list *fifo)
{
	return (list_is_empty(fifo));
}

void	*fifo_pull(t_list *fifo)
{
	t_list	*elem;

	if (list_is_empty(fifo))
		return (NULL);
	elem = list_nth(fifo, 1);
	list_del(elem);
	return ((void *)elem);
}

void	fifo_store(void *elem, t_list *fifo)
{
	t_list	*pos;

	pos = (t_list *)elem;
	list_push_front(pos, fifo);
}

bool	fifo_create(size_t size, size_t count, t_list *fifo)
{
	t_list	*elem;
	size_t	i;

	INIT_LIST_HEAD(fifo);
	i = 0;
	while (i < count)
	{
		elem = (void *)malloc(size);
		if (elem == NULL)
			return (false);
		fifo_store(elem, fifo);
		i++;
	}
	return (true);
}

void	fifo_destroy(t_list *fifo)
{
	t_list	*elem;

	while (!list_is_empty(fifo))
	{
		elem = list_nth(fifo, 1);
		list_del(elem);
		free(elem);
	}
}

void	data_teardown(t_data *data)
{
	data->fd = -1;
	data->offset = 0;
	data->size = 0;
}

void	data_update(char *s, size_t size, t_data *data)
{
	if (data->size + size > MSG_SIZE_MAX)
	{
		memcpy(data->bytes + data->size, s, MSG_SIZE_MAX - data->size - 1);
		data->bytes[MSG_SIZE_MAX - 1] = '$';
		data->bytes[MSG_SIZE_MAX] = '\0';
		data->size = MSG_SIZE_MAX;
	}
	else
	{
		memcpy(data->bytes + data->size, s, size);
		data->bytes[data->size + size] = '\0';
		data->size += size;
	}

	LOG_DEBUG("data {%.*s}", (int)data->size, data->bytes);
}

static void	data_prefix(char *prefix, size_t prefix_size, t_data *data)
{
	size_t	size;

	if (data->size > MSG_SIZE_MAX - prefix_size)
	{
		size = MSG_SIZE_MAX - prefix_size;
		data->bytes[size] = '$';
		data->size = MSG_SIZE_MAX;
	}
	else
	{
		size = data->size;
		data->size += prefix_size;
	}
	memmove(data->bytes + prefix_size, data->bytes, size);
	memcpy(data->bytes, prefix, prefix_size);
	data->bytes[data->size] = '\0';
	LOG_DEBUG("data {%.*s}", (int)data->size, data->bytes);
}

#define NL	"\n"
void	data_success(t_data *data)
{
	data_prefix(FTP_SUCCESS " ", sizeof(FTP_SUCCESS " ") - 1, data);
	data_update(MSG(NL), data);
}

void	data_error(t_data *data)
{
	data_prefix(FTP_ERROR " ", sizeof(FTP_ERROR " ") - 1, data);
	data_update(MSG(NL), data);
}

void	data_send(t_user *user)
{
	user->nextfds = WFDS;
}

char	*fds_tostring(t_fds fds)
{
	static char	*tostring[] = {
		[NOFDS] = "NOFDS",
		[WFDS] = "WFDS",
		[RFDS] = "RFDS",
		[EFDS] = "EFDS"
	};
	int			i;

	i = (int)fds;
	if (i < 0 || i >= sizeof(tostring) / sizeof(tostring[0]))
		return ("fds_tostring Error");
	return (tostring[i]);
}

t_user	*user_new(t_fifo *fifo)
{
	t_user	*user;
	t_list	*pos;

	if (fifo_empty(&fifo->users))
		LOG_FATAL("fifo->users is empty");
	if (fifo_empty(&fifo->datas))
		LOG_FATAL("fifo->datas is empty");
	user = fifo_pull(&fifo->users);
	user->used = true;
	user->sock = -1;
	user->name[0] = '\0';
	user->home[0] = '\0';
	user->pwd[0] = '\0';
	user->fds = NOFDS;
	user->nextfds = NOFDS;
	user->data.bytes = fifo_pull(&fifo->datas);
	data_teardown(&user->data);
	INIT_LIST_HEAD(&user->set);
	return (user);
}

void	user_del(t_fifo *fifo, t_user *user)
{
	EASY_DEBUG_IN;
	list_del(&user->set);
	socket_close(user->sock);
	fifo_store(user->data.bytes, &fifo->datas);
	fifo_store(user, &fifo->users);
}

void	user_show(t_user *user)
{
	printf("User: %s\nhome: %s\npwd %s\nfds: %s nextfds %s\n", user->name, user->home, user->pwd, fds_tostring(user->fds), fds_tostring(user->nextfds));
	if (user->data.fd == -1)
		printf("data_type DATA_TYPE_MSG\ndata {%.*s}\n", (int)user->data.size, user->data.bytes);
	else
		printf("data_type DATA_TYPE_FILE\nfile fd %d offset %zu size %zu\n", user->data.fd, user->data.offset, user->data.size);
}

void	set_teardown(t_set *set)
{
	INIT_LIST_HEAD(&set->users);
	FD_ZERO(&set->fds);
}

void	sets_teardown(t_set *sets)
{
	set_teardown(&sets[NOFDS]);
	set_teardown(&sets[RFDS]);
	set_teardown(&sets[WFDS]);
	set_teardown(&sets[EFDS]);
}

void	set_move(t_user *user, t_set *sets)
{
	EASY_DEBUG_IN;
	LOG_DEBUG("user->fds %s nextfds %s", fds_tostring(user->fds), fds_tostring(user->nextfds));
	user->fds = user->nextfds;
	list_move(&user->set, &sets[user->fds].users);
	user->nextfds = NOFDS;
	EASY_DEBUG_OUT;
}

void	set_prepare(t_set *set, int *nfds)
{
	t_user	*user;
	t_list	*pos;

	FD_ZERO(&set->fds);
	pos = &set->users;
	while ((pos = pos->next) != &set->users)
	{
		user = CONTAINER_OF(pos, t_user, set);
		FD_SET(user->sock, &set->fds);
		LOG_DEBUG("add sock %d to %s", user->sock, fds_tostring(user->fds));
		if (user->sock > *nfds - 1)
			*nfds = user->sock + 1;
	}
}

int		sets_prepare(int listen, t_set *sets)
{
	int		nfds;
	size_t	i;

	LOG_DEBUG("WFDS: %zu", list_size(&sets[WFDS].users));
	LOG_DEBUG("RFDS: %zu", list_size(&sets[RFDS].users));
	LOG_DEBUG("EFDS: %zu", list_size(&sets[EFDS].users));
	nfds = 0;
	set_prepare(&sets[WFDS], &nfds);
	set_prepare(&sets[RFDS], &nfds);
	set_prepare(&sets[EFDS], &nfds);
	FD_SET(listen, &sets[RFDS].fds);
	if (listen > nfds - 1)
		nfds = listen + 1;
	return (nfds);
}

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
	strncpy(ftp->home, home, PATH_SIZE_MAX);
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
		perror("accept");
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
			perror("send");
		socket_close(sock);
		return (false);
	}
	user->sock = sock;
	strncpy(user->home, ftp->home, PATH_SIZE_MAX);
	strncpy(user->pwd, ftp->home, PATH_SIZE_MAX);
	data_success(&user->data);
	user->nextfds = WFDS;
	set_move(user, ftp->sets);
	return (true);
}

bool	request_user(t_ftp *ftp, t_user *user, int argc, char **argv)
{
	(void)ftp;
	(void)argc;
	if (argc > 2)
	{
		data_update(FTP_ERROR_INVALID_ARGUMENT, &user->data);
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
		strncpy(user->name, argv[1], USER_NAME_SIZE_MAX);
	}
	return (true);
}

bool	request_quit(t_ftp *ftp, t_user *user, int argc, char **argv)
{
	(void)ftp;
	(void)argc;
	(void)argv;
	send(user->sock, MSG(FTP_SUCCESS "\n"), MSG_DONTWAIT);
	user->used = false;
	return (true);
}

#include <sys/utsname.h>	/* uname() */
bool	request_syst(t_ftp *ftp, t_user *user, int argc, char **argv)
{
	struct utsname	buf;

	(void)ftp;
	(void)argc;
	(void)argv;
	if (uname(&buf))
	{
		perror("uname");
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
	(void)argc;
	(void)argv;
	data_update(user->pwd, strlen(user->pwd), &user->data);
	return (true);
}

void	get_path(char *pwd, char *in_path, size_t size_max, char *path)
{
	if (in_path[0] == '/')
	{
		strncpy(path, in_path, PATH_SIZE_MAX);
	}
	else
	{
		strncpy(path, pwd, PATH_SIZE_MAX);
		strncat(path, "/", PATH_SIZE_MAX);
		strncat(path, in_path, PATH_SIZE_MAX);
	}
}

#include <dirent.h>
bool	request_pasv(t_ftp *ftp, t_user *user, int argc, char **argv)
{
	DIR				*dp;
	struct dirent	*ep;
	char			path[PATH_SIZE_MAX + 1];

	(void)ftp;
	if (argc > 2)
	{
		data_update(FTP_ERROR_INVALID_ARGUMENT, &user->data);
		return (false);
	}

	if (argv[1] == NULL)
		strncpy(path, user->pwd, PATH_SIZE_MAX);
	else
		get_path(user->pwd, argv[1], PATH_SIZE_MAX, path);

	if ((dp = opendir(path)) == NULL)
		PERROR_FATAL("opendir");

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
		PERROR_FATAL("closedir");

	return (true);
}

bool	request_cwd(t_ftp *ftp, t_user *user, int argc, char **argv)
{
	char	path[PATH_SIZE_MAX + 1];

	(void)ftp;
	if (argv[1] == NULL)
		strncpy(path, user->home, PATH_SIZE_MAX);
	else
		get_path(user->pwd, argv[1], PATH_SIZE_MAX, path);

	if (chdir(path))
	{
		LOG_ERROR("chdir failed path {%s}", path);
		return (false);
	}

	if (!getcwd(user->pwd, PATH_SIZE_MAX))
		PERROR_FATAL("getcwd");

	return (request_pwd(ftp, user, argc, argv));
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
	size_t		size;

	get_path(user->pwd, file, PATH_SIZE_MAX, path);
	LOG_DEBUG("open {%s} for reading", path);

	fd = open(path, O_RDONLY);
	if (fd == -1)
	{
		perror("open");
		data_update(MSG("Failed to open file: "), &user->data);
		data_update(path, strlen(path), &user->data);
		return (false);
	}
	if (fstat(fd, &sb))
	{
		perror("fstat");
		close(fd);
		data_update(MSG("fstat failed"), &user->data);
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

	map = mmap(0, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (map == MAP_FAILED)
	{
		perror("mmap");
		close(fd);
		return (false);
	}

	fifo_store(user->data.bytes, &ftp->fifo.datas);

	user->data.fd = fd;
	user->data.bytes = map;
	user->data.offset = 0;
	user->data.size = sb.st_size;

	return (true);
}

bool	open_file_for_writing(char *file, size_t size, t_ftp *ftp, t_user *user)
{
	char		path[PATH_SIZE_MAX + 1];
	int			fd;
	struct stat	sb;
	char		*map;

	get_path(user->pwd, file, PATH_SIZE_MAX, path);
	LOG_DEBUG("open {%s} for writing", path);

	fd = open(path, O_CREAT | O_TRUNC | O_RDWR, 0777);
	if (fd == -1)
	{
		perror("open");
		data_update(MSG("Failed to open file: "), &user->data);
		data_update(path, strlen(path), &user->data);
		return (false);
	}

	if (ftruncate(fd, size))
	{
		perror("ftruncate");
		close(fd);
		data_update(MSG("Couldnt create a file of the given size"), &user->data);
		return (false);
	}

	map = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (map == MAP_FAILED)
	{
		perror("mmap");
		close(fd);
		data_update(MSG("mmap failed"), &user->data);
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
		perror("munmap");

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
	char		size_str[FILE_SIZE_MAX + 1];
	size_t		size_str_index;
	size_t		size_str_size;

	ASSERT (open_file_for_reading(argv[1], ftp, user));
	size = user->data.size;
	if (size == 0)
	{
		send(user->sock, "0\n", sizeof("0\n") - 1, MSG_DONTWAIT);
		return (true);
	}
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

	LOG_DEBUG("file size {%.*s}", (int)size_str_size, size_str + size_str_index);

	send(user->sock, size_str + size_str_index, size_str_size, MSG_DONTWAIT);

	return (true);
}

bool	request_put(t_ftp *ftp, t_user *user, int argc, char **argv)
{
	ASSERT (open_file_for_writing(argv[1], atoi(argv[2]), ftp, user));
	return (true);
}

typedef struct	s_request
{
	char	*str;
	bool	(*func)(t_ftp *, t_user *, int, char **);
	size_t	expect_argc;
	t_fds	fds;
}				t_request;

bool	request(t_ftp *ftp, t_user *user, int argc, char **argv)
{
	static t_request	requests[] = {
		{ "PWD",  request_pwd,  (size_t)1,  WFDS },
		{ "USER", request_user, (size_t)-1, WFDS },
		{ "QUIT", request_quit, (size_t)1,  NOFDS },
		{ "SYST", request_syst, (size_t)1,  WFDS },
		{ "PASV", request_pasv, (size_t)-1, WFDS },
		{ "CWD",  request_cwd,  (size_t)-1, WFDS },
		{ "GET",  request_get,  (size_t)2,  WFDS },
		{ "PUT",  request_put,  (size_t)3,  RFDS }
	};
	size_t				i;

	i = 0;
	while (i < sizeof(requests) / sizeof(requests[0]))
	{
		if (!strcmp(argv[0], requests[i].str))
		{
			if (requests[i].expect_argc != (size_t)-1 &&
				argc != requests[i].expect_argc)
			{
				data_update(FTP_ERROR_INVALID_ARGUMENT, &user->data);
				return (false);
			}
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
	*argv = strsplit_whitespace(data);
	if (!*argv)
		return (false);
	*argc = 0;
	while ((*argv)[*argc])
		(*argc) += 1;
	return (*argc > 0 ? true : false);
}

bool	request_init(t_ftp *ftp, t_user *user, char *data, size_t size)
{
	int		argc;
	char	**argv;
	bool	status;
	size_t	i;

	data[size] = '\0';
	ASSERT (request_split(data, &argv, &argc));
	data_teardown(&user->data);
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
	size = recv(user->sock, data, MSG_SIZE_MAX, MSG_DONTWAIT);
	if (size == 0)
	{
		user->used = false;
		return (true);
	}
	if (size < 0)
	{
		perror("recv");
		if (user->data.fd != -1)
		{
			close_file(ftp, user);
		}
		return (false);
	}

	LOG_DEBUG("recv {%.*s}", (int)size, data);

	if (user->data.fd == -1)
	{
		request_init(ftp, user, data, (size_t)size);
	}
	else
	{
		nb_write = (user->data.offset + size > user->data.size) ?
			user->data.size - user->data.offset : size;

		LOG_DEBUG("write {%.*s} at offset {%zu} size max {%zu}", (int)nb_write, data, user->data.offset, user->data.size);
		memcpy(user->data.bytes + user->data.offset, data, nb_write);
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
		perror("send");
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
			PERROR_FATAL("select");
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
	port = atoi(argv[2]);
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
