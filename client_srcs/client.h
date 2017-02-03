#ifndef CLIENT_H
# define CLIENT_H

# include "buf.h"
# include "get_next_line.h"
# include <unistd.h>
# include <stdbool.h>

/*
** Utils
*/

int		write_data(const int fd, const char *cmd, const unsigned int size);
void	free_strarray(char **array);
char	**split_cmd(t_buf *cmd);

int		send_msg(const int sock, char *msg, size_t size);

/*
** local
*/
bool	islocal(t_buf *cmd);
int		local(t_buf *cmd);

/*
** file transfer
*/
bool	isfiletransfer(t_buf *cmd);
int		init_file_transfer(const int sock, t_gnl *gnl_sock, t_buf *cmd);
int		server_accept_transfer(t_gnl *gnl_sock, t_buf *msg, bool *ok);

int		send_file(const int sock, t_gnl *gnl_sock, char **argv);
int		recv_file(const int sock, t_gnl *gnl_sock, char **argv, t_buf *cmd);

#endif
