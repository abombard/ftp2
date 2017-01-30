#include "server.h"
#include "strerror.h"
#include <stdlib.h>

extern void		clear_data(t_data *data)
{
	INIT_LIST_HEAD(&data->list);
	data->fd = -1;
	data->bytes = NULL;
	data->offset = 0;
	data->size = 0;
	data->size_max = 0;
}

extern t_data	*alloc_data(void)
{
	t_data	*data;

	data = (t_data *)malloc(sizeof(t_data));
	if (!data)
	{
		perror("malloc", errno);
		return (NULL);
	}
	clear_data(data);
	return (data);
}

extern t_data	*alloc_data_msg(size_t size)
{
	t_data	*data;

	data = malloc(sizeof(t_data) + size + 1);
	if (!data)
	{
		perror("malloc", errno);
		return (NULL);
	}
	clear_data(data);
	data->bytes = (void *)data + sizeof(t_data);
	data->size_max = size;
	return (data);
}

