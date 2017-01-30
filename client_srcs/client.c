#include "client.h"
#include "strerror.h"
#include "log.h"
#include "libft.h"
#include "sock.h"
#include "get_next_line.h"

#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

void	free_strarray(char **array)
{
	int	i;

	i = 0;
	while (array[i])
	{
		free(array[i++]);
	}
	free(array);
}

int		send_msg(const int sock, char *msg, size_t size)
{
	size_t	nsend;
	ssize_t	nbyte;

	nsend = 0;
	while (nsend < size)
	{
		nbyte = send(sock, msg, size, 0);
		if (nbyte < 0)
			return (errno);
		nsend += (size_t)nbyte;
	}
	return (ESUCCESS);
}

/*
** utils
*/
char	**split_cmd(t_buf *cmd)
{
	char	**argv;

	cmd->bytes[cmd->size] = '\0';
	argv = strsplit_whitespace(cmd->bytes);
	cmd->bytes[cmd->size] = '\n';
	return (argv);
}

/*
** local command
*/
bool	islocal(t_buf *cmd)
{
	return (cmd->size > 1 && cmd->bytes[0] == ':');
}

bool	isbuiltin(char *cmd)
{
	return (!ft_strcmp(cmd, ":cd") ||
			!ft_strcmp(cmd, ":pwd"));
}

int		move_directory(char *path)
{
	if (chdir(path))
		return (errno);
	return (0);
}

int		print_directory()
{
	char	path[255 + 1];

	if (!getcwd(path, 255))
		return (errno);
	write_data(1, path, ft_strlen(path));
	write_data(1, "\n", sizeof("\n") - 1);
	return (0);
}

int		exec_builtin(char **argv)
{
	int	err;

	err = 0;
	if (!ft_strcmp(argv[0], ":cd"))
		err = move_directory(argv[1]);
	if (!ft_strcmp(argv[0], ":pwd"))
		err = print_directory();
	return (err);
}

#include <unistd.h>
#include <sys/wait.h>

int		exec_cmd(char **argv)
{
	pid_t	pid;

	pid = fork();
	if (pid == -1)
		return (errno);
	if (pid == 0)
	{
		execv(argv[0], argv);
		perror(argv[0], errno);
		exit(1);
	}
	wait(NULL);
	return (0);
}

int		local(t_buf *cmd)
{
	char	**argv;
	char	*c;
	int		err;

	argv = split_cmd(cmd);
	if (!argv)
		return (errno);
	if (isbuiltin(argv[0]))
	{
		err = exec_builtin(argv);
	}
	else
	{
		if ((c = ft_strjoin("/usr/bin/", argv[0] + 1)))
		{
			free(argv[0]);
			argv[0] = c;
			err = exec_cmd(argv);
		}
		else
		{
			err = errno;
		}
	}
	free_strarray(argv);
	return (err);
}

/*
** file transfer
*/
bool	isfiletransfer(t_buf *cmd)
{
	char	c[3];

	if (cmd->size < 3)
		return (false);
	c[0] = ft_toupper(cmd->bytes[0]);
	c[1] = ft_toupper(cmd->bytes[1]);
	c[2] = ft_toupper(cmd->bytes[2]);
	if (ft_memcmp("PUT", c, sizeof(c)) &&
		ft_memcmp("GET", c, sizeof(c)))
		return (false);
	ft_memcpy(cmd->bytes, c, sizeof(c));
	return (true);
}

#include <sys/stat.h>
int		file_size(char *file, size_t *size)
{
	struct stat	sb;

	if (lstat(file, &sb))
	{
		perror("lstat", errno);
		return (errno);
	}
	*size = (size_t)sb.st_size;
	return (0);
}

static int	send_file_init(const int sock, char *file_path, size_t size)
{
	char	msg[255 + 1];
	char	*file;
	char	*file_size;
	int		err;

	file = ft_strrchr(file_path, '/');
	file = file ? file + 1 : file_path;
	file_size = ft_itoa(size);
	ft_strncpy(msg, "put", 255);
	ft_strncat(msg, " ", 255);
	ft_strncat(msg, file, 255);
	ft_strncat(msg, " ", 255);
	ft_strncat(msg, file_size, 255);
	ft_strncat(msg, "\n", 255);
	err = send_msg(sock, msg, ft_strlen(msg));
	free(file_size);
	return (err);
}

static int	server_accept_transfer(t_gnl *gnl_sock, t_buf *msg, bool *ok)
{
	if (!get_next_line(gnl_sock, msg))
		return (ECONNABORTED);
	if (msg->size >= sizeof("ERROR") - 1 &&
		!ft_memcmp(msg->bytes, "ERROR", sizeof("ERROR") - 1))
		*ok = false;
	else
		*ok = true;
	return (0);
}

#include <fcntl.h>
#include <sys/mman.h>
int		send_file(const int sock, t_gnl *gnl_sock, char **argv)
{
	int		fd;
	char	*addr;
	size_t	size;
	int		err;

	LOG_DEBUG("sending file %s", argv[1]);

	fd = open(argv[1], O_RDONLY);
	if (fd == -1)
		return (errno);

	err = file_size(argv[1], &size);
	if (err)
	{
		close(fd);
		return (err);
	}

	err = send_file_init(sock, argv[1], size);
	if (err)
	{
		close(fd);
		return (err);
	}

	if (size == 0)
	{
		close(fd);
		return (0);
	}

	addr = mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (addr == MAP_FAILED)
	{
		err = errno;
		close(fd);
		return (err);
	}

	t_buf	msg;
	bool	ok;
	err = server_accept_transfer(gnl_sock, &msg, &ok);
	if (!err)
	{
		if (ok)
			err = send_msg(sock, addr, size);
		else
			write_data(1, msg.bytes, msg.size);
	}

	munmap(addr, size);
	close(fd);
	return (err);
}

static int	get_file_size(t_buf *msg, size_t *size)
{
	char	**argv;
	int		file_size;

	msg->bytes[msg->size] = '\0';
	argv = strsplit_whitespace(msg->bytes);
	if (!argv)
		return (errno);
	if (!argv[1])
	{
		free_strarray(argv);
		return (EBADE);
	}
	file_size = ft_atoi(argv[1]);
	if (file_size < 0)
	{
		free_strarray(argv);
		return (EBADE);
	}
	free_strarray(argv);
	*size = (size_t)file_size;
	return (0);
}

int		recv_file(const int sock, t_gnl *gnl_sock, char **argv, t_buf *cmd)
{
	int		fd;
	char	*map;
	size_t	size;

	LOG_DEBUG("recv_file: %s", argv[1]);

	if (argv[1] == NULL)
		return (EINVAL);

	fd = open(argv[1], O_CREAT | O_EXCL | O_RDWR, 0777);
	if (fd == -1)
		return (errno);

	int		err;

	err = send_msg(sock, cmd->bytes, cmd->size + 1);
	if (err)
		return (err);

	t_buf	msg;
	bool	ok;

	err = server_accept_transfer(gnl_sock, &msg, &ok);
	if (err)
	{
		close(fd);
		return (err);
	}
	if (!ok)
	{
		write_data(1, msg.bytes, msg.size);
		write_data(1, "\n", sizeof("\n") - 1);
		close(fd);
		return (0);
	}

	err = get_file_size(&msg, &size);
	if (err)
	{
		close(fd);
		return (err);
	}

	if (size == 0)
	{
		close(fd);
		return (0);
	}

	if (ftruncate(fd, size))
	{
		err = errno;
		close(fd);
		return (err);
	}

	map = mmap(0, size, PROT_WRITE, MAP_SHARED, fd, 0);
	if (map == MAP_FAILED)
	{
		err = errno;
		close(fd);
		return (err);
	}

	size_t	nwritten;
	ssize_t	nread;

	gnl_flush(gnl_sock, &msg);
	LOG_DEBUG("gnl_flush {%.*s}", (int)msg.size, msg.bytes);

	ft_memcpy(map, msg.bytes, msg.size > size ? size : msg.size);

	nwritten = msg.size;
	while (nwritten < size)
	{
		nread = read(sock, map + nwritten, size - nwritten);
		if (nread < 0)
		{
			err = errno;
			break ;
		}
		if (nread == 0)
		{
			err = ECONNABORTED;
			break ;
		}
		nwritten += (size_t)nread;
	}

	munmap(map, size);
	close(fd);
	return (err);
}

int		init_file_transfer(const int sock, t_gnl *gnl_sock, t_buf *cmd)
{
	char	**argv;
	int		err;

	argv = split_cmd(cmd);
	if (!argv)
		return (errno);
	err = 0;
	if (!ft_strcmp(argv[0], "PUT"))
	{
		err = send_file(sock, gnl_sock, argv);
	}
	if (!ft_strcmp(argv[0], "GET"))
	{
		err = recv_file(sock, gnl_sock, argv, cmd);
	}
	if (err)
		perror(argv[0], err);
	free_strarray(argv);
	return (0);
}

/*
** loop
*/

int		simple_output(t_gnl *gnl_sock, t_buf *msg)
{
	if (!get_next_line(gnl_sock, msg))
		return (ECONNABORTED);
	write_data(1, msg->bytes, msg->size);
	write_data(1, "\n", sizeof("\n") - 1); 
	return (0);
}

#define PROMPT		"$> "
int		client_loop(const int sock)
{
	t_gnl	gnl_stdin;
	t_gnl	gnl_sock;
	t_buf	msg;
	int		err;

	gnl_init(&gnl_sock, sock);
	gnl_init(&gnl_stdin, 0);
	while (write_data(1, PROMPT, sizeof(PROMPT) - 1), get_next_line(&gnl_stdin, &msg))
	{
		if (islocal(&msg))
			err = local(&msg);
		else if (isfiletransfer(&msg))
			err = init_file_transfer(sock, &gnl_sock, &msg);
		else
		{
			err = send_msg(sock, msg.bytes, msg.size + 1);
			if (err)
				return (err);
			err = simple_output(&gnl_sock, &msg);
		}
		if (err)
			return (err);
	}
	return (0);
}

int		main(int argc, char **argv)
{
	int		sock;
	char	*host;
	int		port;
	int		err;

	if (argc != 3)
	{
		LOG_ERROR("Usage: %s server port", argv[0]);
		return (1);
	}
	host = argv[1];
	port = ft_atoi(argv[2]);
	sock = query_connection(host, port);
	if (sock == -1)
		return (1);
	err = client_loop(sock);
	if (err && err != ECONNABORTED)
		perror("client_loop", err);
	close(sock);
	return (err);
}
