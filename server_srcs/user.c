#include "user.h"
#include "sock.h"

char	*fds_tostring(t_fds fds)
{
	static char	*tostring[] = {
		[NOFDS] = "NOFDS",
		[WFDS] = "WFDS",
		[RFDS] = "RFDS",
		[EFDS] = "EFDS"
	};
	int			i;

	i = (int)fds;
	if (i < 0 || (size_t)i >= sizeof(tostring) / sizeof(tostring[0]))
		return ("fds_tostring Error");
	return (tostring[i]);
}

t_user	*user_new(t_fifo *fifo)
{
	t_user	*user;

	if (fifo_empty(&fifo->users))
		return (false);
	if (fifo_empty(&fifo->datas))
		return (false);
	user = fifo_pull(&fifo->users);
	user->used = true;
	user->sock = -1;
	user->name[0] = '\0';
	user->home[0] = '\0';
	user->pwd[0] = '\0';
	user->fds = NOFDS;
	user->nextfds = NOFDS;
	user->data.bytes = fifo_pull(&fifo->datas);
	data_teardown(&user->data);
	INIT_LIST_HEAD(&user->set);
	return (user);
}

void	user_del(t_fifo *fifo, t_user *user)
{
	list_del(&user->set);
	socket_close(user->sock);
	fifo_store(user->data.bytes, &fifo->datas);
	fifo_store(user, &fifo->users);
}

#include <stdio.h> //TEMP
void	user_show(t_user *user)
{
	printf("User: %s\nhome: %s\npwd %s\nfds: %s nextfds %s\n", user->name, user->home, user->pwd, fds_tostring(user->fds), fds_tostring(user->nextfds));
	if (user->data.fd == -1)
		printf("data_type DATA_TYPE_MSG\ndata {%.*s}\n", (int)user->data.size, user->data.bytes);
	else
		printf("data_type DATA_TYPE_FILE\nfile fd %d offset %zu size %zu\n", user->data.fd, user->data.offset, user->data.size);
}
