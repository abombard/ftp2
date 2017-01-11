#ifndef LOG_H
# define LOG_H

#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>			/* size_t */

#define LOG_DEBUG(...) private_log(__FILE__, __func__, __LINE__, "DEBUG", __VA_ARGS__)

#define LOG_ERROR(...) private_log(__FILE__, __func__, __LINE__, "ERROR", __VA_ARGS__)
#define LOG_WARNING(...) private_log(__FILE__, __func__, __LINE__, "WARNING", __VA_ARGS__)

#define LOG_FATAL(...) do { LOG_ERROR(__VA_ARGS__); return ( false ); } while (0)
#define PERROR_FATAL(func) do { perror(func); return ( false ); } while (0)

#define ASSERT(expr) do { if (! (expr)) LOG_FATAL (#expr " failed"); } while (0)

#define SYS_ERR(expr) do {		\
	int error_number = (expr);	\
	if (error_number) {			\
		errno = error_number;	\
		perror (#expr);			\
		return ( false );		\
	}							\
} while ( 0 )

# define LOG_MSG_SIZE_MAX		2047

bool	write_data (const int fd, const char *cmd, const size_t size);
void	private_log(const char *file,
					const char *func,
					const int line,
					const char *logtype,
					const char *fmt, ...) __attribute__((format(printf,5,6)));

#endif
