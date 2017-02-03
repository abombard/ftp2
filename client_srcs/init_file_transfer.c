#include "client.h"
#include "get_next_line.h"
#include "buf.h"
#include "strerror.h"
#include "libft.h"

int		init_file_transfer(const int sock, t_gnl *gnl_sock, t_buf *cmd)
{
	char	**argv;
	int		err;

	argv = split_cmd(cmd);
	if (!argv)
		return (errno);
	err = 0;
	if (!ft_strcmp(argv[0], "PUT"))
	{
		err = send_file(sock, gnl_sock, argv);
	}
	if (!ft_strcmp(argv[0], "GET"))
	{
		err = recv_file(sock, gnl_sock, argv, cmd);
	}
	if (err)
		perror(argv[0], err);
	free_strarray(argv);
	return (0);
}
