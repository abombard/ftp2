#ifndef REQUEST_H
# define REQUEST_H

typedef struct	s_request
{
	char	*str;
	int		(*func)(int, char **, t_user *, t_io *);
}				t_request;

#endif
