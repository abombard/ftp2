#ifndef SERVER_H
# define SERVER_H

# include "printf.h"

# include "sys_types.h"
# include "list.h"

# include <sys/select.h>

# define SUCCESS				"SUCCESS"
# define SUCCESS_SIZE	(sizeof("SUCCESS") - 1)
# define ERROR					"ERROR"
# define ERROR_SIZE		(sizeof("ERROR") - 1)

# define MSG(msg)	(msg), (sizeof(msg) - 1)

/*
** data
*/
typedef struct	s_data
{
	t_list		list;

	int			fd;
	char		*bytes;
	size_t		offset;
	size_t		size;
	size_t		size_max;

}				t_data;

/*
** io
*/
typedef struct	s_io
{
	t_list	list;

	bool	connected;
	int		sock;
	t_data	data_in;
	char	*input_buffer;
	t_list	datas_out;
}				t_io;

/*
** user
*/
# define NAME_SIZE_MAX	63
# define PATH_SIZE_MAX	255
typedef struct	s_user
{
	char	name[NAME_SIZE_MAX + 1];
	char	home[PATH_SIZE_MAX + 1];
	char	pwd[PATH_SIZE_MAX + 1];
}				t_user;

/*
** server
*/
# define CONNECTION_COUNT_MAX		63
# define DATA_COUNT_MAX				(CONNECTION_COUNT_MAX * 2)
# define INPUT_BUFFER_SIZE			255

# define WFDS		0
# define RFDS		1
# define EFDS		2
# define SET_COUNT	3

typedef struct	s_server
{
	char	*host;
	int		port;

	int		listen;

	int		sig_warn;

	t_list	io_list;
	t_io	io_array[CONNECTION_COUNT_MAX];

	t_user	user_array[CONNECTION_COUNT_MAX];

	fd_set	fds[SET_COUNT];

	char	home[PATH_SIZE_MAX + 1];
}				t_server;

/*
** Utils
*/
extern int		move_directory(t_user *user, char *path, size_t size);
extern size_t	concat_safe(char *buf, size_t offset, size_t size_max, char *s);
extern void		concat_path(char *pwd, char *in_path, char *path);

/*
** Data
*/
extern void		clear_data(t_data *data);
extern t_data	*alloc_data(size_t size);
extern void		free_file(t_data *data);
extern void		free_data(t_data *data);

extern int		file_create(char *path, size_t size, t_data *file);
extern int		file_load(char *path, t_data *file);

extern bool		send_data(t_server *server, t_io *io);
extern bool		read_data(t_server *server, t_io *io);

/*
** msg
*/
extern t_data	*msg_success(char *msg);
extern t_data	*msg_error(char *msg);

/*
** user
*/
extern t_user	*get_user(int sock, t_server *server);
extern void		user_init(t_user *user, char *home);

/*
** io
*/
extern t_io		*get_io(int sock, t_server *server);
extern void		io_input_teardown(t_io *io);
extern int		create_io(int sock, t_server *server);
extern void		delete_io(t_io *io);
extern void		foreach_io(t_server *server, bool (*io_func)(t_server *, t_io *));

/*
** set
*/
extern void		sets_prepare(t_server *server, int *nfds);

/*
** server
*/
extern bool		handle_new_connections(t_server *server, bool *new_user);

extern bool		server_open(char *host, int port, char *home, t_server *server);
extern bool		server_loop(t_server *server);
extern bool		server_close(t_server *server);

/*
** requests
*/

extern bool		request(t_server *server, t_io *io);
extern int		request_pwd(int argc, char **argv, t_user *user, t_io *io);
extern int		request_user(int argc, char **argv, t_user *user, t_io *io);
extern int		request_quit(int argc, char **argv, t_user *user, t_io *io);
extern int		request_syst(int argc, char **argv, t_user *user, t_io *io);
extern int		request_ls(int argc, char **argv, t_user *user, t_io *io);
extern int		request_cd(int argc, char **argv, t_user *user, t_io *io);
extern int		request_get(int argc, char **argv, t_user *user, t_io *io);
extern int		request_put(int argc, char **argv, t_user *user, t_io *io);

#endif
