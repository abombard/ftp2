#include "libft.h"
#include "get_next_line.h"
#include "buf.h"
#include "strerror.h"
#include "client.h"
#include <fcntl.h>
#include <sys/mman.h>

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

	if (argv[1] == NULL)
		return (EINVAL);

	int		err;

	err = send_msg(sock, cmd->bytes, cmd->size + 1);
	if (err)
		return (err);

	t_buf	msg;
	bool	ok;

	err = server_accept_transfer(gnl_sock, &msg, &ok);
	if (err)
		return (err);
	if (!ok)
	{
		write_data(1, msg.bytes, msg.size);
		write_data(1, "\n", sizeof("\n") - 1);
		return (0);
	}

	fd = open(argv[1], O_CREAT | O_EXCL | O_RDWR, 0777);
	if (fd == -1)
		return (errno);

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
