#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include "client.h"
#include "libft.h"
#include "strerror.h"
#include "buf.h"

bool	islocal(t_buf *cmd)
{
	return (cmd->size > 1 && cmd->bytes[0] == ':');
}

bool	isbuiltin(char *cmd)
{
	return (!ft_strcmp(cmd, ":cd") ||
			!ft_strcmp(cmd, ":pwd"));
}

int		move_directory(char *path)
{
	if (!path)
		return (EINVAL);
	if (chdir(path))
		return (errno);
	return (0);
}

int		print_directory()
{
	char	path[255 + 1];

	if (!getcwd(path, 255))
		return (errno);
	write_data(1, path, ft_strlen(path));
	write_data(1, "\n", sizeof("\n") - 1);
	return (0);
}

int		exec_builtin(char **argv)
{
	int	err;

	err = 0;
	if (!ft_strcmp(argv[0], ":cd"))
		err = move_directory(argv[1]);
	if (!ft_strcmp(argv[0], ":pwd"))
		err = print_directory();
	return (err);
}

#include <unistd.h>
#include <sys/wait.h>

int		exec_cmd(char **argv)
{
	pid_t	pid;

	pid = fork();
	if (pid == -1)
		return (errno);
	if (pid == 0)
	{
		execv(argv[0], argv);
		perror(argv[0], errno);
		exit(1);
	}
	wait(NULL);
	return (0);
}

int		local(t_buf *cmd)
{
	char	**argv;
	char	*c;
	int		err;

	argv = split_cmd(cmd);
	if (!argv)
		return (errno);
	if (isbuiltin(argv[0]))
	{
		err = exec_builtin(argv);
	}
	else
	{
		if ((c = ft_strjoin("/bin/", argv[0] + 1)))
		{
			free(argv[0]);
			argv[0] = c;
			err = exec_cmd(argv);
		}
		else
		{
			err = errno;
		}
	}
	if (err)
		perror(argv[0], err);
	free_strarray(argv);
	return (0);
}
