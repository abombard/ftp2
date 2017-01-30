#ifndef LOG_H
# define LOG_H

#define LOG_DEBUG(...) private_log(__FILE__, __func__, __LINE__, "DEBUG", __VA_ARGS__)

#define LOG_ERROR(...) private_log(__FILE__, __func__, __LINE__, "ERROR", __VA_ARGS__)
#define LOG_WARNING(...) private_log(__FILE__, __func__, __LINE__, "WARNING", __VA_ARGS__)

#define LOG_FATAL(...) do { LOG_ERROR(__VA_ARGS__); return ( false ); } while (0)
#define PERROR_FATAL(func) do { perror(func); return ( false ); } while (0)

#define ASSERT(expr) do { if (! (expr)) LOG_FATAL (#expr " failed"); } while (0)

# define LOG_MSG_SIZE_MAX		5000

int		write_data (const int fd, const char *cmd, const unsigned int size);
void	private_log(const char *file,
					const char *func,
					const int line,
					const char *logtype,
					const char *fmt, ...) __attribute__((format(printf,5,6)));

#endif
