/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   data.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/02/03 16:58:40 by abombard          #+#    #+#             */
/*   Updated: 2017/02/03 17:01:05 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"
#include "strerror.h"
#include <stdlib.h>
#include <sys/mman.h>

extern void		clear_data(t_data *data)
{
	INIT_LIST_HEAD(&data->list);
	data->fd = -1;
	data->bytes = NULL;
	data->offset = 0;
	data->size = 0;
	data->size_max = 0;
}

extern t_data	*alloc_data(size_t size)
{
	t_data	*data;

	data = malloc(sizeof(t_data) + size);
	if (!data)
	{
		perror("malloc", errno);
		return (NULL);
	}
	clear_data(data);
	data->bytes = (void *)data + sizeof(t_data);
	data->size_max = size - 1;
	return (data);
}

extern void		free_file(t_data *data)
{
	if (munmap(data->bytes, data->size_max))
		perror("munmap", errno);
	if (close(data->fd))
		perror("close", errno);
}

extern void		free_data(t_data *data)
{
	list_del(&data->list);
	if (data->fd != -1)
		free_file(data);
	free(data);
}
