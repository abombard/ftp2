#include "server.h"
#include "listen_socket.h"

#include "sys_types.h"
#include "strerror.h"
#include "libft.h"
#include "list.h"
#include "printf.h"

#include <signal.h>

void	perror(char *s, int err)
{
	ft_fprintf(2, "%s: %s\n", s, strerror(err));
}

/*
** utils
*/
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

/*
** msg
*/
# define MSG(msg) (msg), (sizeof(msg) - 1)

void	msg_clear(t_msg *msg)
{
	msg->type = MSG_TYPE_UNDEFINED;
	msg->fd = -1;
	msg->bytes = NULL;
	msg->offset = 0;
	msg->size = 0;
	msg->size_max = 0;
}

t_msg	*int_new_msg(void)
{
	t_msg	*msg;

	msg = malloc(sizeof(t_msg));
	if (!msg)
	{
		perror("malloc", errno);
		return (NULL);
	}
	msg_clear(msg);
	return (msg);
}

t_msg	*new_msg(void)
{
	t_msg	*msg;

	msg = int_new_msg();
	if (!msg)
		return (NULL);
	msg->type = MSG_TYPE_MSG;
	msg->bytes = malloc(MSG_SIZE_MAX + 1);
	if (!msg->bytes)
	{
		perror("malloc", errno);
		free(msg);
		return (NULL);
	}
	msg->size_max = MSG_SIZE_MAX;
	return (msg);
}

void	del_msg(t_msg *msg)
{
	if (msg->type == MSG_TYPE_MSG)
	{
		if (close(msg->fd))
			perror("close", errno);
		if (munmap(msg->bytes, msg->size_max) == -1)
			perror("munmap", errno);
	}
	else if (msg->type == MSG_TYPE_FILE)
	{
		free(msg->bytes);
	}
	free(msg);
}

void	msg_add(t_msg *msg, char *s)
{
	msg->size = concat_safe(msg->bytes, msg->size, msg->size_max, s);
}

t_msg	*msg_create(char *s)
{
	t_msg	*msg;

	msg = new_msg();
	if (!msg)
		return (NULL);
	msg_add(msg, s);
	return (msg);
}

t_msg	*msg_err(char *s, int err)
{
	t_msg	*msg;

	msg = new_msg();
	if (!msg)
		return (NULL);
	msg_add(msg, s);
	msg_add(msg, ": ");
	msg_add(msg, strerror(err));
	return (msg);
}

# include <fcntl.h>
# include <sys/stat.h>
# include <sys/mman.h>
t_msg	*new_file_send(char *file)
{
	t_msg	*msg;
	int		fd;
	char	*map;

	fd = open(file, O_RDONLY);
	if (fd == -1)
	{
		msg = msg_err("open", errno);
		return (msg);
	}
	if (fstat(fd, &sb))
	{
		msg = msg_err("fstat", errno);
		close(fd);
		return (msg);
	}
	if (!S_ISREG(sb.st_mode))
	{
		msg = msg_err(file, EISDIR);
		close(fd);
		return (msg);
	}
	if (sb.st_size == 0)
	{
		msg = msg_create("SUCCESS");
		close(fd);
		return (msg);
	}

	map = mmap(0, (size_t)sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (map == MAP_FAILED)
	{
		msg = msg_err("mmap", errno);
		close(fd);
		return (msg);
	}

	msg = int_msg_new();
	if (!msg)
	{
		munmap(map, sb.st_size);
		close(fd);
		return (NULL);
	}

	msg->type = MSG_TYPE_FILE;
	msg->fd = fd;
	msg->bytes = map;
	msg->offset = 0;
	msg->size = (size_t)sb.st_size;
	msg->size_max = (size_t)sb.st_size;

	return (msg);
}

t_msg	*new_file_recv(char *file, size_t size)
{
	t_msg	*msg;
	int		fd;
	char	*map;

	fd = open(path, O_CREAT | O_TRUNC | O_RDWR, 0777);
	if (fd == -1)
	{
		msg = msg_err("open" , errno);
		return (msg);
	}

	if (size == 0)
	{
		msg = msg_create("SUCCESS");
		close(fd);
		return (msg);
	}

	if (ftruncate(fd, (off_t)size))
	{
		msg = msg_err("ftruncate", errno);
		close(fd);
		return (msg);
	}

	map = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (map == MAP_FAILED)
	{
		msg = msg_err("mmap", errno);
		close(fd);
		return (msg);
	}

	msg = int_msg_new();
	if (!msg)
	{
		munmap(map);
		close(fd);
		return (NULL);
	}

	msg->type = MSG_FILE;
	msg->fd = fd;
	msg->bytes = map;
	msg->offset = 0;
	msg->size = 0;
	msg->size_max = size;

	return (msg);
}

void	msg_sub(t_msg *msg, size_t index, size_t size)
{
	ft_memmove(msg->bytes, msg->bytes + index, msg->size);
	msg->size = size;
}

/*
** file
*/
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

bool	send_file(char *file, t_user *user)
{
	t_msg		*msg;
	char		path[PATH_SIZE_MAX + 1];

	get_path(user->pwd, file, PATH_SIZE_MAX, path);
	LOG_DEBUG("open {%s} for reading", path);

	msg = new_file_send(file);
	if (!msg)
		return (false);
	list_add_tail(&msg->list, &user->io.msgs_out);

	return (true);
}

bool	recv_file(char *file, size_t size, t_user *user)
{
	t_msg		*msg;
	char		path[PATH_SIZE_MAX + 1];

	get_path(user->pwd, file, PATH_SIZE_MAX, path);
	LOG_DEBUG("open {%s} for writing", path);

	msg = new_file_recv(path, size);
	if (!msg)
		return (false);
	if (msg->type == MSG_TYPE_FILE)
	{
		msg_add(msg, user->io->in.bytes, user->io->in.size);
		del_msg(user->in);
		user->in = msg;
	}

	return (true);
}

bool	read_data(int sock, t_msg *msg, ssize_t *nbyte)
{
	*nbyte = recv(sock, msg->bytes + msg->size, msg->size_max - msg->size, MSG_DONTWAIT);
	if (*nbyte < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return (0);
		perror("recv", errno);
		return (false);
	}
	if (*nbyte == 0)
	{
		//TEMP
		if (msg->size == msg->size_max)
			return (true);
	}
	LOG_DEBUG("recv {%.*s}", (int)nbyte, msg->bytes + msg->size);
	msg->size += (size_t)*nbyte;
	return (true);
}

bool	write_data(int sock, t_msg *msg, ssize_t *nbyte)
{
	*nbyte = send(msg->sock, msg->bytes + msg->offset, msg->size - msg->offset, MSG_DONTWAIT);
	if (*nbyte < 0)
	{
		perror("send", errno);
		return (false);
	}
	LOG_DEBUG("send {%.*s}", (int)size, msg->bytes);
	msg->offset += (size_t)size;
	return (true);
}

/*
** io
*/
void	io_init(t_io *io, int sock)
{
	INIT_LIST_HEAD(&io->fifo);
	io->online = false;
	io->sock = sock;
	INIT_LIST_HEAD(&io->msgs_in);
	INIT_LIST_HEAD(&io->msgs_out);
}

/*
** user
*/
bool	users_teardown(t_user users[], size_t count)
{
	size_t	i;

	i = 0;
	while (i < count)
	{
		io_init(&users[i].io, i);
		i++;
	}
	return (true);
}

bool	foreach_user(t_user users[], size_t count, bool (*func)(t_user *))
{
	t_user	*user;
	size_t	fail_count;
	size_t	i;

	fail_count = 0;
	i = 0;
	while (i < count)
	{
		user = &users[i];
		if (user->online)
		{
			if (!func(user))
				fail_count++;
		}
		i++;
	}
	return (fail_count == 0);
}

t_user	*user_pull(t_server *server, int sock)
{
	t_user	*user;
	void	*map;
	size_t	map_size;

	if (sock >= USER_COUNT_MAX)
		return (NULL);
	user = &server->users[sock];

	ft_strncpy(user->home, server->home, PATH_SIZE_MAX);
	ft_strncpy(user->pwd, server->home, PATH_SIZE_MAX);

	list_add(&user->io, &server->select.ios);

	return (user);
}

bool	add_user(t_server *server)
{
	t_user	*user;
	int		sock;
	ssize_t	size;

	sock = accept_connection(server->listen);
	if (sock == -1)
		return (false);
	user = user_pull(server->users, sock);
	if (user == NULL)
	{
		size = send(sock, MSG("ERROR Too many connections\n"), MSG_DONTWAIT);
		if (size == -1)
			LOG_ERROR("send: %s", strerror(errno));
		socket_close(sock);
		return (false);
	}
	return (true);
}

bool	del_user(t_user *user)
{
	socket_close(user->sock);
	user->online = false;
	return (true);
}

/*
** sets
*/
void	set_teardown(t_set *set)
{
	INIT_LIST_HEAD(&set->msgs);
}

void	sets_teardown(t_set sets[])
{
	set_teardown(sets[RFDS]);
	set_teardown(sets[WFDS]);
	set_teardown(sets[EFDS]);
}

/*
** sets prepare
*/
static void	fds_set(t_list *ios, fd_set *fds, int *nfds)
{
	t_io	*io;
	t_list	*pos;

	pos = ios;
	while ((pos = pos->next) != ios)
	{
		io = CONTAINER_OF(pos, t_io, list);
		FD_SET(io->sock, &fds[RFDS]);
		if (!list_empty(&io->msgs_out))
			FD_SET(io->sock, &fds[WFDS]);
		if (io->sock >= *nfds)
			*nfds = io->sock + 1;
	}
}

void		sets_prepare(t_select *sel, int *nfds)
{
	FD_ZERO(&sel->fds[RFDS]);
	FD_ZERO(&sel->fds[WFDS]);
	FD_ZERO(&sel->fds[EFDS]);

	FD_SET(server->listen, &sel->fds[RFDS]);
	*nfds = server->listen + 1;

	fds_set(&sel->ios, sel->fds, nfds);
}

/*
** update datas
*/
void	update_datas(t_select *sel)
{
	t_io	*io;
	t_list	*pos;
	ssize_t	nbyte;

	pos = &sel->ios;
	while ((pos = pos->next) != &sel->ios)
	{
		io = CONTAINER_OF(pos, t_io, list);
		if (FD_ISSET(io->sock, sel->fds[RFDS]))
		{
			if (!read_data(io->sock, io->in, &nbyte))
			{
			
			}
		}
		if (FD_ISSET(io->sock, sel->fds[WFDS]))
		{
			t_msg	*msg;
			t_list	*n;

			n = list_nth(&io->msgs_out, 1);
			msg = CONTAINER_OF(n, t_msg, list);
			if (!send_data(io->sock, msg, &nbyte))
			{

			}
		}
	}
}

/*
** request
*/
bool	request_user(int argc, char **argv, t_user *user, t_msg *out)
{
	if (argc > 2)
	{
		msg_add(out, strerror(E2BIG));
		return (false);
	}
	if (argc == 1)
	{
		if (strlen(user->name) == 0)
		{
			msg_add(out, "User did not registered");
			return (false);
		}
		msg_add(out, user->name);
	}
	else if (argc == 2)
	{
		ft_strncpy(user->name, argv[1], NAME_SIZE_MAX);
	}
	return (true);
}

bool	request_quit(int argc, char **argv, t_user *user, t_msg *out)
{
	(void)argv;
	if (argc > 1)
	{
		msg_add(out, strerror(EARGS));
		return (false);
	}
	send(user->msg_w.sock, MSG(SUCCESS "\n"), MSG_DONTWAIT);
	del_user(user);
	return (true);
}

#include <sys/utsname.h>	/* uname() */
bool	request_syst(int argc, char **argv, t_user *user, t_msg *out)
{
	struct utsname	buf;

	(void)argv;
	(void)user;
	if (argc > 1)
	{
		msg_add(out, strerror(E2BIG));
		return (false);
	}
	if (uname(&buf))
	{
		msg_add(out, strerror(errno));
		return (false);
	}
	msg_add(out, buf.sysname);
	msg_add(out, " ");
	msg_add(out, buf.release);
	return (true);
}

bool	request_pwd(int argc, char **argv, t_user *user, t_msg *out)
{
	(void)argv;
	if (argc > 1)
	{
		msg_add(out, strerror(E2BIG));
		return (false);
	}
	msg_add(out, user->pwd);
	return (true);
}

#include <dirent.h>
bool	request_ls(int argc, char **argv, t_user *user, t_msg *out)
{
	DIR				*dp;
	struct dirent	*ep;
	char			path[PATH_SIZE_MAX + 1];

	char			*map;
	off_t			offset;
	off_t			size;

	if (argc > 2)
		return (E2BIG);

	size = getpagesize();
	offset = 0;
	map = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, 0, offset);
	if (map == MAP_FAILED)
	{
		msg_add(out, strerror(errno));
		return (false);
	}

	if (argv[1] == NULL)
		ft_strncpy(path, user->pwd, PATH_SIZE_MAX);
	else
		get_path(user->pwd, argv[1], PATH_SIZE_MAX, path);

	if ((dp = opendir(path)) == NULL)
	{
		msg_add(out, strerror(errno));
		munmap(map, size);
		return (false);
	}

	while ((ep = readdir(dp)) != NULL)
	{
		if (ep->d_name[0] == '.')
			continue ;
		offset = concat_safe(map, offset, size, ep->d_name);
		offset = concat_safe(map, offset, size, " ");
	}
	if (offset > 0)
		offset -= 1;

	closedir(dp);

	out->type = MSG_FILE;
	out->bytes = map;
	out->offset = 0;
	out->size = offset;
	out->size_max = size;

	return (true);
}

bool	request_cd(int argc, char **argv, t_user *user, t_msg *out)
{
	char	*pwd_argv[2];
	char	path[PATH_SIZE_MAX + 1];

	if (argc > 2)
	{
		msg_add(out, strerror(E2BIG));
		return (false);
	}

	if (argc == 1)
		ft_strncpy(path, user->home, PATH_SIZE_MAX);
	else
		get_path(user->pwd, argv[1], PATH_SIZE_MAX, path);

	if (chdir(path))
	{
		msg_add(out, strerror(errno));
		return (false);
	}

	if (!getcwd(user->pwd, PATH_SIZE_MAX))
	{
		msg_add(out, strerror(errno));
		return (false);
	}

	pwd_argv[0] = "PWD";
	pwd_argv[1] = NULL;
	return (request_pwd(1, pwd_argv, user, out));
}

# define FILE_SIZE_MAX	64
bool	request_get(int argc, char **argv, t_user *user, t_msg *out)
{
	size_t		size;
	char		size_str[FILE_SIZE_MAX + 1 + (sizeof(SUCCESS " ") - 1)];
	size_t		size_str_index;
	size_t		size_str_size;

	if (argc != 2)
	{
		msg_add(out, strerror(EARGS));
		return (false);
	}
	ASSERT (send_file(argv[1], user));
	size = out->size;
	if (size == 0)
	{
		size_str_index = 0;
		size_str_size = sizeof(SUCCESS " 0\n") - 1;
		ft_memcpy(size_str, SUCCESS " 0\n", size_str_size);
	}
	else
	{
		size_str_index = FILE_SIZE_MAX;
		size_str[size_str_index--] = '\n';
		size_str_size = 1;
		while (size > 0 && size_str_index > sizeof(SUCCESS " ") - 1)
		{
			size_str[size_str_index--] = size % 10 + '0';
			size /= 10;
			size_str_size++;
		}
		size_str_index++;
		size_str_index -= sizeof(SUCCESS " ") - 1;
		size_str_size += sizeof(SUCCESS " ") - 1;
		ft_memcpy(size_str + size_str_index,
				SUCCESS " ", sizeof(SUCCESS " ") - 1);
	}

	LOG_DEBUG("file size {%.*s}", (int)size_str_size, size_str + size_str_index);

	send(out->sock, size_str + size_str_index, size_str_size, 0);

	return (true);
}

bool	request_put(int argc, char **argv, t_user *user, t_msg *out)
{
	int	size;

	if (argc != 3)
	{
		msg_add(out, strerror(EARGS));
		return (false);
	}
	size = atoi(argv[2]);
	if (size < 0)
	{
		msg_add(out, strerror(EINVAL));
		return (false);
	}
	ASSERT (recv_file(argv[1], (size_t)size, user));
	return (true);
}

typedef struct	s_request
{
	char	*str;
	bool	(*io_data)(int, char **, t_user *, t_msg *);
}				t_request;

bool	launch_request(int argc, char **argv, t_user *user, t_msg *out)
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
		if (!strcmp(argv[0], requests[i].str))
			return (requests[i].io_data(argc, argv, user, out));
		i++;
	}
	msg_add(out, strerror(EBADRQC));
	return (false);
}

bool	get_request(t_msg *in, t_buf *request)
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

void	free_array(char ***argv)
{
	int	i;

	i = 0;
	while ((*argv)[i])
	{
		free((*argv)[i]);
		i++;
	}
	free(*argv);
}

bool	handle_request(t_buf *request, t_user *user)
{
	int		argc;
	char	**argv;
	t_msg	out;
	bool	success;

	ASSERT (split_request(request, &argc, &argv));

	msg_init(&out, user->msg_w.sock);
	success = launch_request(argc, argv, user, &out);
	if (success)
	{

	}
	else
	{

	}

	free_array(&argv);
	return (true);
}

bool	check_msg_r(t_user *user)
{
	t_msg	*msg;

	msg = &user->msg_r;
	if (msg->type != MSG_BUF && msg->type != MSG_FILE)
	{
		LOG_ERROR("msg->type %d", msg->type);
		return (false);
	}

	if (msg->type == MSG_FILE)
	{
		if (msg->size == msg->size_max)
		{
			close_file(msg);
			msg_clear(msg);
			msg_add(&user->msg_w, SUCCESS);
		}
	}
	else if (msg->type == MSG_BUF)
	{
		t_msg	out;
		t_buf	request;

		if (user->msg_w.type == MSG_FILE)
			return (true);

		msg_init(&out, user->msg_w.sock);
		while (get_request(msg, &request))
		{
			LOG_DEBUG("new request {%.*s}", (int)request.size, request.bytes);
			handle_request(&request, user, &out);
			msg_sub(msg, request.size + 1, msg->size - (request.size + 1));
		}

		if (msg->size == msg->size_max)
		{
			msg_clear(msg);
			msg_add(&user->msg_w, strerror(EMSGSIZE));
		}
	}

	return (true);
}

