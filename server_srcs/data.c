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
	if (data->size == data->size_max)
	{
		data->bytes[data->size_max - 1] = '$';
		return ;
	}
	if (data->size + size > data->size_max)
	{
		ft_memcpy(data->bytes + data->size, s, data->size_max - data->size - 1);
		data->bytes[data->size_max - 1] = '$';
		data->size = data->size_max - 1;
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
	if (data->size + prefix_size > data->size_max)
	{
		data->size = data->size_max - prefix_size - 1;
		data->bytes[data->size_max - 1] = '$';
	}
	ft_memmove(data->bytes + prefix_size, data->bytes, data->size);
	ft_memcpy(data->bytes, prefix, prefix_size);
	data->size += prefix_size;
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
