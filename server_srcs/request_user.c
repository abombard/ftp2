#include "server.h"
#include "strerror.h"
#include "libft.h"

extern int	request_user(int argc, char **argv, t_user *user, t_io *io)
{
	t_data	*data;

	data = NULL;
	if (argc > 2)
		return (E2BIG);
	if (argc == 1)
	{
		if (ft_strlen(user->name) == 0)
			return (ENOTREGISTER);
		data = msg_success(user->name);
		if (!data)
			return (errno);
		list_add_tail(&data->list, &io->datas_out);
	}
	else if (argc == 2)
	{
		ft_strncpy(user->name, argv[1], NAME_SIZE_MAX);
		data = msg_success(user->name);
		if (!data)
			return (errno);
		list_add_tail(&data->list, &io->datas_out);
	}
	return (0);
}
