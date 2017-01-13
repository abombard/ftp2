#include "set.h"
#include "list.h"

void	set_teardown(t_set *set)
{
	INIT_LIST_HEAD(&set->users);
	FD_ZERO(&set->fds);
}

void	sets_teardown(t_set *sets)
{
	set_teardown(&sets[NOFDS]);
	set_teardown(&sets[RFDS]);
	set_teardown(&sets[WFDS]);
	set_teardown(&sets[EFDS]);
}

void	set_move(t_user *user, t_set *sets)
{
	user->fds = user->nextfds;
	if (user->nextfds == RFDS)
		data_teardown(&user->data);
	list_move(&user->set, &sets[user->fds].users);
	user->nextfds = NOFDS;
}

void	set_prepare(t_set *set, int *nfds)
{
	t_user	*user;
	t_list	*pos;

	FD_ZERO(&set->fds);
	pos = &set->users;
	while ((pos = pos->next) != &set->users)
	{
		user = CONTAINER_OF(pos, t_user, set);
		FD_SET(user->sock, &set->fds);
		if (user->sock > *nfds - 1)
			*nfds = user->sock + 1;
	}
}

int		sets_prepare(int listen, t_set *sets)
{
	int		nfds;

	nfds = 0;
	set_prepare(&sets[WFDS], &nfds);
	set_prepare(&sets[RFDS], &nfds);
	set_prepare(&sets[EFDS], &nfds);
	FD_SET(listen, &sets[RFDS].fds);
	if (listen > nfds - 1)
		nfds = listen + 1;
	return (nfds);
}
