#include "server.h"
#include "libft.h"

extern t_user	*get_user(int sock, t_server *server)
{
	if (sock < 0 ||
(size_t)sock >= sizeof(server->user_array) / sizeof(server->user_array[0]))
		return (NULL);
	return (&server->user_array[sock]);
}

extern void		user_init(t_user *user, char *home)
{
	user->name[0] = '\0';
	ft_strncpy(user->home, home, PATH_SIZE_MAX);
	ft_strncpy(user->pwd, home, PATH_SIZE_MAX);
}
