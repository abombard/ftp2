#include "client.h"
#include "get_next_line.h"
#include "log.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <stdio.h>

int		service_create(const char *server_name, const int port)
{
	int					sock;
	struct sockaddr_in	sockaddr;
	struct hostent		*server_info;

	ft_memset(&sockaddr, 0, sizeof(sockaddr));
	server_info = gethostbyname(server_name);
	if (server_info == NULL)
	{
		herror("gethostbname");
		return (-1);
	}
	sockaddr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)server_info->h_addr)));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);

	sock = socket(sockaddr.sin_family, SOCK_STREAM, 0);
	if (sock == -1)
	{
		perror("socket");
		return (-1);
	}

	if (connect(sock, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr)) < 0)
	{
		perror("connect");
		close(sock);
		return (-1);
	}

	return (sock);
}

#define MSG_SIZE_MAX	1023

bool	session_open(const int sock)
{
	char	buf[MSG_SIZE_MAX + 1];
	ssize_t	size;

	size = recv(sock, buf, MSG_SIZE_MAX, 0);
	if (size < 0)
	{
		perror("recv");
		return (false);
	}
	buf[size] = '\0';
	if (ft_memcmp(buf, "SUCCESS", sizeof("SUCCESS") - 1))
	{
		LOG_ERROR("Failed to connect");
		return (false);
	}
	return (true);
}

bool	ready_for_writting(const int sock, int *ready)
{
	fd_set			fds;
	struct timeval	tv;

	FD_ZERO(&fds);
	FD_SET(sock, &fds);
	tv.tv_sec = 1000;
	tv.tv_usec = 0;
	*ready = select(sock + 1, NULL, &fds, NULL, &tv);
	if (*ready < 0)
	{
		perror("select");
		return (false);
	}
	return (true);
}

bool	send_msg(const int sock, char *msg, size_t size)
{
	int		ready;
	ssize_t	nbyte;

	ready = 0;
	while (!ready)
	{
		if (!ready_for_writting(sock, &ready))
			return (false);
	}
	while (size > 0)
	{
		nbyte = send(sock, msg, size, 0);
		if (nbyte < 0)
		{
			perror("send");
			return (false);
		}
		if ((int)size - nbyte < 0)
		{
			LOG_ERROR("send_msg: size - nbyte %ld", (ssize_t)size - nbyte);
			return (false);
		}
		msg += nbyte;
		size -= (size_t)nbyte;
	}
	return (true);
}

bool	ready_for_reading(const int sock, int *ready)
{
	fd_set			fds;
	struct timeval	tv;

	FD_ZERO(&fds);
	FD_SET(sock, &fds);
	tv.tv_sec = 1000;
	tv.tv_usec = 0;
	*ready = select(sock + 1, &fds, NULL, NULL, &tv);
	if (*ready < 0)
	{
		perror("select");
		return (false);
	}
	return (true);
}

bool	recv_msg(const int sock, size_t size_max, char *msg, size_t *size)
{
	int		ready;
	ssize_t	nread;

	ready = 0;
	while (!ready)
	{
		if (!ready_for_reading(sock, &ready))
			return (false);
	}
	nread = recv(sock, msg, size_max, 0);
	if (nread < 0)
	{
		perror("recv");
		return (false);
	}
	*size = (size_t)nread;
	return (true);
}

bool	split_cmd(char *cmd, char ***av, int *ac)
{
	*av = strsplit_whitespace(cmd);
	if (*av == NULL)
		return (false);
	*ac = 0;
	while ((*av)[*ac])
		*ac += 1;
	return (*ac != 0);
}

static bool	send_file_send(const int sock, const char *file,
						char *addr, size_t size)
{
	char	cmd[MSG_SIZE_MAX + 1];
	char	*file_size;
	bool	status;

	file_size = ft_itoa((int)size);
	if (file_size == NULL)
	{
		LOG_ERROR("ft_itoa failed size %zu", size);
		return (false);
	}
	ft_strncpy(cmd, "PUT ", MSG_SIZE_MAX);
	ft_strncat(cmd, file, MSG_SIZE_MAX);
	ft_strncat(cmd, " ", sizeof(" ") - 1);
	ft_strncat(cmd, file_size, MSG_SIZE_MAX);
	ft_strncat(cmd, "\n", sizeof("\n") - 1);
	status =
		send_msg(sock, cmd, (size_t)ft_strlen(cmd)) &&
		send_msg(sock, addr, size);
	free(file_size);
	return (status);
}

#include <sys/mman.h>
static bool	send_file_map(const int sock,
			const char *file, const int fd, size_t size)
{
	char	*addr;
	bool	status;

	addr = mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (addr == MAP_FAILED)
	{
		perror("mmap");
		return (false);
	}
	status = send_file_send(sock, file, addr, size);
	if (munmap(addr, size))
	{
		perror("munmap");
		return (false);
	}
	return (status);
}

#include <fcntl.h>
#include <sys/stat.h>
static bool	send_file_open(const int sock, char *file)
{
	int			fd;
	struct stat	sb;
	bool		status;

	fd = open(file, O_RDONLY);
	if (fd == -1)
	{
		perror("open");
		return (false);
	}
	if (fstat(fd, &sb))
	{
		perror("fstat");
		close(fd);
		return (false);
	}
	if (sb.st_size < 0)
	{
		LOG_ERROR("file size %ld", sb.st_size);
		return (false);
	}
	status = send_file_map(sock, file, fd, (size_t)sb.st_size);
	if (close(fd))
	{
		perror("close");
		return (false);
	}
	return (true);
}

bool	send_file(const int sock, char *cmd)
{
	int		ac;
	char	**av;
	bool	status;
	int		i;

	if (!split_cmd(cmd, &av, &ac))
	{
		LOG_ERROR("split_cmd failed");
		return (false);
	}
	if (ac != 2)
	{
		LOG_ERROR("Invalid number of arguments %d expected 2", ac);
		return (true);
	}
	status = send_file_open(sock, av[1]);
	i = 0;
	while (i < ac)
		free(av[i++]);
	free(av);
	return (status);
}

//TEMP
bool	recv_file(const int sock, char *cmd)
{
	int		ac;
	char	**av;
	bool	status;
	int		i;

	if (!split_cmd(cmd, &av, &ac))
	{
		LOG_ERROR("split_cmd failed");
		return (false);
	}
	if (ac != 2)
	{
		LOG_ERROR("Invalid number of arguments");
		return (false);
	}
	//status = recv_file_open(sock, av[1]);
	(void)sock;
	status = false;
	i = 0;
	while (i < ac)
		free(av[i++]);
	free(av);
	return (status);
}

#define PROMPT				"$> "
#define PROMPT_SIZE	(sizeof("$> ") - 1)

bool	client_loop(const int sock)
{
	t_gnl	gnl;
	char	buf_in[MSG_SIZE_MAX + 1];
	t_buf	msg_in;
	char	buf_out[MSG_SIZE_MAX + 1];
	size_t	size;

	gnl_init(&gnl, 0, buf_in, sizeof(buf_in));
	while (write_data(1, PROMPT, PROMPT_SIZE), get_next_line(&gnl, &msg_in))
	{
		if (msg_in.size == 0)
			continue ;

		if (!ft_memcmp(msg_in.bytes, "PUT", sizeof("PUT") - 1) ||
			!ft_memcmp(msg_in.bytes, "put", sizeof("put") - 1))
		{
			msg_in.bytes[msg_in.size] = '\0';
			if (!send_file(sock, msg_in.bytes))
				continue ;
		}
		else
		{
			msg_in.bytes[msg_in.size] = '\n';
			if (!send_msg(sock, msg_in.bytes, msg_in.size + 1))
				continue ;
		}

		if (!recv_msg(sock, MSG_SIZE_MAX, buf_out, &size))
			return (false);

		write_data(1, buf_out, size);

		if (size == 0 || !ft_memcmp(msg_in.bytes, "QUIT", sizeof("QUIT") - 1))
			break ;
	}

	return (true);
}

int		main(int ac, char **av)
{
	int		sock;
	char	*server_name;
	int		port;
	int		status;

	if (ac != 3)
	{
		LOG_ERROR("Usage: %s server port", av[0]);
		return (1);
	}
	server_name = av[1];
	port = ft_atoi(av[2]);
	sock = service_create(server_name, port);
	if (sock == -1)
		return (1);
	if (!session_open(sock))
	{
		LOG_ERROR("session_open failed");
		return (1);
	}
	status = client_loop(sock) ? 0 : 1;
	close(sock);
	return (status);
}
