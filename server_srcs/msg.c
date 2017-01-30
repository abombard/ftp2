#include "server.h"
#include "libft.h"

static t_data	*msg_create(char *status, char *msg)
{
	t_data	*data;
	size_t	size;

	size = ft_strlen(status) + sizeof(" ") - 1 + ft_strlen(msg) + 1;
	data = alloc_data_msg(size);
	if (!data)
		return (NULL);
	ft_strncpy(data->bytes, status, data->size_max);
	ft_strncat(data->bytes, " ", data->size_max);
	ft_strncat(data->bytes, msg, data->size_max);
	data->size = size;
	data->bytes[data->size] = '\n';
	data->size += 1;
	return (data);
}

extern t_data	*msg_success(char *msg)
{
	return (msg_create("SUCCESS", msg));
}

extern t_data	*msg_error(char *msg)
{
	return (msg_create("ERROR", msg));
}
