#include "server.h"
#include "log.h"
#include "libft.h"
#include "printf.h"
#include <stdlib.h>

static char	*sgetenv(char **environ, char *search)
{
	size_t	search_size;
	char	*cur;
	size_t	cur_size;
	int		i;

	search_size = ft_strlen(search);
	i = 0;
	while (environ[i])
	{
		cur = environ[i];
		cur_size = ft_strlen(environ[i]);
		if (cur_size > search_size &&
			!ft_memcmp(cur, search, search_size) &&
			cur[search_size] == '=')
			return (cur + search_size + 1);
		i++;
	}
	return (NULL);
}

#include <unistd.h>
int		main(int argc, char **argv, char **environ)
{
	t_server	server;
	char		*host;
	int			port;
	char		*home;
	int			exit_status;

	if (argc != 3)
	{
		ft_fprintf(2, "Usage: %s host port\n", argv[0]);
		return (EXIT_FAILURE);
	}
	host = argv[1];
	port = ft_atoi(argv[2]);
	home = sgetenv(environ, "HOME");
	if (!server_open(host, port, home, &server))
	{
		LOG_ERROR("server_open failed host {%s} port {%s}", argv[1], argv[2]);
		return (EXIT_FAILURE);
	}
	exit_status = EXIT_SUCCESS;
	while (1)
	{
		if (!server_loop(&server))
		{
			exit_status = EXIT_FAILURE;
			break ;
		}
	}
	if (!server_close(&server))
	{
		LOG_ERROR("server_close failed");
		return (EXIT_FAILURE);
	}
	return (exit_status);
}
