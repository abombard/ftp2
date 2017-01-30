#ifndef SOCK_H
# define SOCK_H

int		open_socket(const int ai_family,
					const int ai_socktype,
					const int ai_protocol);

void 	close_socket(int sock);

int		query_connection(char *host, int port);
int		accept_connection(int listen_socket);

#endif
