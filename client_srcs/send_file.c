#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include "strerror.h"
#include "libft.h"
#include "client.h"

static int	file_size(char *file, size_t *size)
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

extern int	send_file(const int sock, t_gnl *gnl_sock, char **argv)
{
	int		fd;
	size_t	size;
	int		err;

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
		close(fd);
		return (0);
	}

	if (size == 0)
	{
		close(fd);
		return (0);
	}

	char	*addr;

	addr = mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (addr == MAP_FAILED)
	{
		err = errno;
		close(fd);
		return (err);
	}

	err = send_msg(sock, addr, size);

	munmap(addr, size);
	close(fd);
	return (err);
}
