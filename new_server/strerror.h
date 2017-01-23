#ifndef STRERROR_H
# define STRERROR_H

#include <errno.h>

#define __MACOSX__

# ifdef __LINUX__
# define EARGS	(int)(EXFULL + 1)
# endif
# ifdef __MACOSX__
# define EARGS	(int)(EQFULL + 1)
# endif

char	*strerror(int err_num);

#endif
