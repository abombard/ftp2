#include "buf.h"
#include "libft.h"
#include <stdbool.h>

bool	isfiletransfer(t_buf *cmd)
{
	char	c[3];

	if (cmd->size < 3)
		return (false);
	c[0] = ft_toupper(cmd->bytes[0]);
	c[1] = ft_toupper(cmd->bytes[1]);
	c[2] = ft_toupper(cmd->bytes[2]);
	if (ft_memcmp("PUT", c, sizeof(c)) &&
		ft_memcmp("GET", c, sizeof(c)))
		return (false);
	ft_memcpy(cmd->bytes, c, sizeof(c));
	return (true);
}
