#ifndef STRERROR_H
# define STRERROR_H

#include <errno.h>

# define EARGS	(int)(EXFULL + 1)

char	*strerror(int err_num);

#endif
