#include "server.h"
#include "strerror.h"
#include "libft.h"
#include <sys/utsname.h>

extern int		request_syst(int argc, char **argv, t_user *user, t_io *io)
{
	t_data			*data;
	char			buf[128];
	struct utsname	ubuf;

	(void)argv;
	(void)user;
	if (argc > 1)
		return (E2BIG);
	if (uname(&ubuf))
		return (errno);
	ft_strncpy(buf, ubuf.sysname, sizeof(buf));
	ft_strncat(buf, " ", sizeof(buf));
	ft_strncat(buf, ubuf.release, sizeof(buf));
	data = msg_success(buf);
	if (!data)
		return (errno);
	list_add_tail(&data->list, &io->datas_out);
	return (0);
}
