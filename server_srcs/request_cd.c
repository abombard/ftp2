#include "server.h"
#include "strerror.h"
#include "libft.h"

extern int		request_cd(int argc, char **argv, t_user *user, t_io *io)
{
	char	path[PATH_SIZE_MAX + 1];
	char	*pwd_argv[2];
	int		err;

	if (argc > 2)
		return (E2BIG);

	if (argc == 1)
		ft_strncpy(path, user->home, PATH_SIZE_MAX);
	else
		concat_path(user->pwd, argv[1], path);
	if ((err = move_directory(user, path, PATH_SIZE_MAX)))
		return (err);
	ft_strncpy(user->pwd, path, PATH_SIZE_MAX);
	pwd_argv[0] = "PWD";
	pwd_argv[1] = NULL;
	return (request_pwd(1, pwd_argv, user, io));
}
