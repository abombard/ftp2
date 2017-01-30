#include "server.h"
#include "libft.h"
#include "strerror.h"

static int	check_permission(char *path, t_user *user)
{
	size_t	size;

	size = ft_strlen(user->home);
	if (ft_memcmp(path, user->home, size) ||
		(path[size] != '/' && path[size] != '\0'))
		return (EACCES);
	return (ESUCCESS);
}

extern int	move_directory(t_user *user, char *path, size_t size)
{
	int		err;

	if (chdir(path))
		return (errno);
	if (!getcwd(path, size))
		return (errno);
	if (!ft_strcmp(user->name, "root"))
		return (0);
	err = check_permission(path, user);
	return (err);
}
