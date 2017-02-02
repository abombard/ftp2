#include "server.h"
#include "strerror.h"
#include "libft.h"

extern int		request_put(int argc, char **argv, t_user *user, t_io *io)
{
	t_data	*data;
	char	path[PATH_SIZE_MAX + 1];
	int		size;
	int		err;

	if (argc != 3)
		return (EINVAL);
	size = ft_atoi(argv[2]);
	if (size < 0)
		return (EINVAL);
	if (ft_strchr(argv[1], '/'))
		return (EINVAL);
	concat_path(user->pwd, argv[1], path);
	err = file_create(path, size, &io->data_in);
	if (err)
		return (err);
	data = msg_success("");
	if (!data)
		return (errno);
	list_add_tail(&data->list, &io->datas_out);
	return (0);
}

