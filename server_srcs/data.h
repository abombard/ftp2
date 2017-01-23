#ifndef DATA_H
# define DATA_H

# include "sys_types.h"

# define MSG_SIZE_MAX	1023
# define MSG(str) (str), (sizeof(str) - 1)

typedef enum	e_fds
{
	NOFDS = 0,
	RFDS,
	WFDS,
	EFDS
}				t_fds;

char	*fds_tostring(t_fds fds);

typedef enum	s_data_state
{
	STATE_UNDEFINED,
	STATE_COMMAND,
	STATE_FILE_TRANSFER
}				t_data_state;

typedef struct	s_file
{
	int		fd;
	char	*bytes;
	size_t	offset;
	size_t	size;
}				t_file;

typedef struct	s_cmd
{
	char	*bytes;
	size_t	offset;
	size_t	size;
	size_t	size_max;
}				t_cmd;

void			data_teardown(t_data *data);
void			data_update(char *s, size_t size, t_data *data);
void			data_success(t_data *data);
void			data_error(t_data *data);
void			data_errno(int err_num, t_data *data);

#endif
