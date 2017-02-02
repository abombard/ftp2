#include "server.h"
#include "strerror.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

static int	get_file_size(int fd, size_t *size)
{
	struct stat	sb;

	if (fstat(fd, &sb))
		return (errno);
	if (!S_ISREG(sb.st_mode))
		return (EFTYPE);
	*size = (size_t)sb.st_size;
	return (0);
}

static int	int_file_load(const int fd, t_data *file)
{
	char	*map;
	size_t	size;
	int		err;

	err = get_file_size(fd, &size);
	if (err)
		return (err);
	if (size == 0)
		return (0);
	map = mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (map == MAP_FAILED)
		return (errno);
	file->fd = fd;
	file->bytes = map;
	file->offset = 0;
	file->size = size;
	file->size_max = size;
	return (0);
}

extern int	file_load(char *path, t_data *file)
{
	int		fd;
	int		err;

	fd = open(path, O_RDONLY);
	if (fd == -1)
		return (errno);
	err = int_file_load(fd, file);
	if (err)
	{
		close(fd);
		return (err);
	}
	if (file->size == 0)
	{
		close(fd);
	}
	return (0);
}
