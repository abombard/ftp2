#include "server.h"
#include "strerror.h"
#include <fcntl.h>
#include <sys/mman.h>

static int	int_file_create(const int fd, size_t size, char **map)
{
	if (ftruncate(fd, size))
		return (errno);
	*map = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (*map == MAP_FAILED)
		return (errno);
	return (0);
}

extern int	file_create(char *path, size_t size, t_data *file)
{
	int		fd;
	char	*map;
	int		err;

	fd = open(path, O_CREAT | O_EXCL | O_RDWR, 0644);
	if (fd == -1)
		return (errno);
	if (size == 0)
	{
		close(fd);
		return (0);
	}
	err = int_file_create(fd, size, &map);
	if (err)
	{
		close(fd);
		return (err);
	}
	file->fd = fd;
	file->bytes = map;
	file->offset = 0;
	file->size = 0;
	file->size_max = size;
	return (0);
}
