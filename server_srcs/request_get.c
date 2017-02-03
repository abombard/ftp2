#include "server.h"
#include "strerror.h"
#include "libft.h"
#include <stdlib.h>

static int		build_answer(size_t size, t_data **answer)
{
	char	*size_str;
	int		err;

	size_str = ft_itoa(size);
	if (!size_str)
		return (errno);
	*answer = msg_success(size_str);
	if (!*answer)
		err = errno;
	else
		err = 0;
	free(size_str);
	return (err);
}

extern int		request_get(int argc, char **argv, t_user *user, t_io *io)
{
	t_data		*file;
	t_data		*answer;
	char		path[PATH_SIZE_MAX + 1];
	int			err;

	if (argc != 2 || ft_strchr(argv[1], '/'))
		return (EINVAL);
	concat_path(user->pwd, argv[1], path);
	if ((file = alloc_data(0)) == NULL)
		return (errno);
	err = file_load(path, file);
	if (err)
	{
		free_data(file);
		return (err);
	}
	err = build_answer(file->size, &answer);
	if (err)
	{
		free_data(file);
		return (err);
	}
	list_add_tail(&answer->list, &io->datas_out);
	list_add_tail(&file->list, &io->datas_out);
	return (0);
}
