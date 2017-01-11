#include "data.h"
#include "sys_types.h"
#include "libft.h"
#include "strerror.h"

#define FTP_SUCCESS	"SUCCESS"
#define FTP_ERROR	"ERROR"

void	data_teardown(t_data *data)
{
	data->fd = -1;
	data->offset = 0;
	data->size = 0;
}

void	data_update(char *s, size_t size, t_data *data)
{
	if (data->size == MSG_SIZE_MAX)
	{
		data->bytes[MSG_SIZE_MAX - 1] = '$';
		return ;
	}
	if (data->size + size > MSG_SIZE_MAX)
	{
		ft_memcpy(data->bytes + data->size, s, MSG_SIZE_MAX - data->size - 1);
		data->bytes[MSG_SIZE_MAX - 1] = '$';
		data->size = MSG_SIZE_MAX - 1;
	}
	else
	{
		ft_memcpy(data->bytes + data->size, s, size);
		data->size += size;
	}
	data->bytes[data->size] = '\0';
}

static void	data_prefix(char *prefix, size_t prefix_size, t_data *data)
{
	size_t	size;

	if (data->size + prefix_size > MSG_SIZE_MAX)
	{
		size = MSG_SIZE_MAX - prefix_size;
		data->bytes[size - 1] = '$';
		data->size = MSG_SIZE_MAX;
	}
	else
	{
		size = data->size;
		data->size += prefix_size;
	}
	ft_memmove(data->bytes + prefix_size, data->bytes, size);
	ft_memcpy(data->bytes, prefix, prefix_size);
	data->bytes[data->size] = '\0';
}

void	data_success(t_data *data)
{
	data_prefix(FTP_SUCCESS " ", sizeof(FTP_SUCCESS " ") - 1, data);
	data->bytes[data->size] = '\n';
	data->size++;
}

void	data_error(t_data *data)
{
	data_prefix(FTP_ERROR " ", sizeof(FTP_ERROR " ") - 1, data);
	data->bytes[data->size] = '\n';
	data->size++;
}

void	data_errno(int err_num, t_data *data)
{
	char	*err;

	data_teardown(data);
	err = strerror(err_num);
	if (err == NULL)
		return ;
	data_update(err, strlen(err), data);
}
