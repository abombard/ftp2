#include "libft.h"
#include "buf.h"

char	**split_cmd(t_buf *cmd)
{
	char	**argv;

	cmd->bytes[cmd->size] = '\0';
	argv = strsplit_whitespace(cmd->bytes);
	cmd->bytes[cmd->size] = '\n';
	return (argv);
}
