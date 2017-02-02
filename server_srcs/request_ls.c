#include "server.h"
#include "strerror.h"
#include "libft.h"
#include <dirent.h>

extern int		request_ls(int argc, char **argv, t_user *user, t_io *io)
{
	DIR				*dp;
	struct dirent	*ep;
	char			path[PATH_SIZE_MAX + 1];

	if (argc > 2)
		return (E2BIG);

	if (argv[1] == NULL)
		ft_strncpy(path, user->pwd, PATH_SIZE_MAX);
	else
		concat_path(user->pwd, argv[1], path);

	int	err;
	err = move_directory(user, path, PATH_SIZE_MAX);
	if (err)
		return (err);

	if ((dp = opendir(path)) == NULL)
		return (errno);

	char			buf[4096];
	off_t			offset;

	offset = 0;
	while ((ep = readdir(dp)) != NULL)
	{
		if (ep->d_name[0] == '.')
			continue ;
		offset = concat_safe(buf, offset, sizeof(buf) - 1, ep->d_name);
		offset = concat_safe(buf, offset, sizeof(buf) - 1, " ");
	}
	if (offset > 0 && buf[offset - 1] != '$')
		offset -= 1;

	buf[offset] = '\0';

	closedir(dp);

	t_data		*data;

	data = msg_success(buf);
	if (!data)
		return (errno);
	list_add_tail(&data->list, &io->datas_out);

	return (0);
}

