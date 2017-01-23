#include "set.h"
#include "list.h"

void	set_teardown(t_set *set)
{
	INIT_LIST_HEAD(&set->datas);
	FD_ZERO(&set->fds);
}

void	sets_teardown(t_set *sets)
{
	set_teardown(&sets[NOFDS]);
	set_teardown(&sets[RFDS]);
	set_teardown(&sets[WFDS]);
	set_teardown(&sets[EFDS]);
}

void	set_prepare(t_set *set, int *nfds)
{
	t_data	*data;
	t_list	*pos;

	FD_ZERO(&set->fds);
	pos = &set->datas;
	while ((pos = pos->next) != &set->datas)
	{
		data = CONTAINER_OF(pos, t_data, set);
		FD_SET(data->sock, &set->fds);
		if (data->sock > *nfds - 1)
			*nfds = data->sock + 1;
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
