#include "get_next_line.h"
#include "buf.h"
#include "libft.h"
#include "strerror.h"
#include <stdbool.h>

int		server_accept_transfer(t_gnl *gnl_sock, t_buf *msg, bool *ok)
{
	if (!get_next_line(gnl_sock, msg))
		return (ECONNABORTED);
	if (msg->size >= sizeof("ERROR") - 1 &&
		!ft_memcmp(msg->bytes, "ERROR", sizeof("ERROR") - 1))
		*ok = false;
	else
		*ok = true;
	return (0);
}
