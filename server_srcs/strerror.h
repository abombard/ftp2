#ifndef STRERROR_H
# define STRERROR_H

#include <errno.h>

#define __LINUX__

# define ESUCCESS		0

# ifdef __LINUX__
# define ENOTREGISTER	(int)(EHWPOISON + 1)
# define EFTYPE			(int)(EHWPOISON + 2)
# endif
# ifdef __MACOSX__
# define ENOTREGISTER	(int)(EQFULL + 1)
# define EBADRQC		(int)(EQFULL + 2)
# endif

char	*strerror(int err_num);
void	perror(char *s, int err);

#endif
