#ifndef DATA_H
# define DATA_H

# include "sys_types.h"

# define MSG_SIZE_MAX	1023
# define MSG(str) (str), (sizeof(str) - 1)

typedef struct	s_data
{
	int		fd;
	char	*bytes;
	size_t	offset;
	size_t	size;
}				t_data;

void			data_teardown(t_data *data);
void			data_update(char *s, size_t size, t_data *data);
void			data_success(t_data *data);
void			data_error(t_data *data);
void			data_errno(int err_num, t_data *data);

#endif
