#ifndef FT_PRINTF_H
# define FT_PRINTF_H

int	ft_printf(const char *fmt, ...) __attribute__((format(printf,1,2)));
int	ft_fprintf(int fd, const char *fmt, \
								...) __attribute__((format(printf,2,3)));

#endif
