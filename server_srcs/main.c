#include "server.h"
#include "libft.h"
#include "printf.h"
#include <stdlib.h>
#include <unistd.h>

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

static bool	server(char *host, int port, char *home)
{
	t_server	server;
	bool		success;

	if (!server_open(host, port, home, &server))
	{
		ft_fprintf(2, "server_open failed\n");
		return (false);
	}
	success = true;
	while (success)
	{
		if (!server_loop(&server))
		{
			success = false;
			break ;
		}
	}
	if (!server_close(&server))
	{
		ft_fprintf(2, "server_close failed\n");
		return (false);
	}
	return (success);
}

int			main(int argc, char **argv, char **environ)
{
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
	exit_status = server(host, port, home) ? EXIT_SUCCESS : EXIT_FAILURE;
	return (exit_status);
}
