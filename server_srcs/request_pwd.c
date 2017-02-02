#include "server.h"
#include "strerror.h"

int	request_pwd(int argc, char **argv, t_user *user, t_io *io)
{
	t_data	*data;

	(void)argv;
	if (argc > 1)
		return (E2BIG);
	data = msg_success(user->pwd);
	if (!data)
		return (errno);
	list_add_tail(&data->list, &io->datas_out);
	return (0);
}
