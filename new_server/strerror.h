#ifndef STRERROR_H
# define STRERROR_H

#include <errno.h>

#define __MACOSX__

# define ESUCCESS		0

# ifdef __LINUX__
# define EARGS			(int)(EXFULL + 1)
# define ENOTREGISTER	(int)(EXFULL + 2)
# endif
# ifdef __MACOSX__
# define EARGS			(int)(EQFULL + 1)
# define ENOTREGISTER	(int)(EQFULL + 2)
# endif

char	*strerror(int err_num);

#endif
