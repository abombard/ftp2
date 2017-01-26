#ifndef STRERROR_H
# define STRERROR_H

#include <errno.h>

#define __LINUX__

# define ESUCCESS		0

# ifdef __LINUX__
# define EARGS			(int)(EHWPOISON + 1)
# define ENOTREGISTER	(int)(EHWPOISON + 2)
# define EFTYPE			(int)(EHWPOISON + 3)
# endif
# ifdef __MACOSX__
# define EARGS			(int)(EQFULL + 1)
# define ENOTREGISTER	(int)(EQFULL + 2)
# define EBADRQC		(int)(EQFULL + 3)
# endif

char	*strerror(int err_num);

#endif
