#ifndef SOCK_H
# define SOCK_H

#include "sys_types.h"
#include "log.h"

#include <sys/types.h>
#include <sys/socket.h>		/* socket(), AF_INET, SOCK_STREAM */
#include <netinet/in.h>
#include <arpa/inet.h>		/* htons */
#include <netdb.h>			/* gethostbyname */

int		open_socket(const int ai_family,
					const int ai_socktype,
					const int ai_protocol);

void 	close_socket(int sock);

int		accept_connection(int listen_socket);

#endif
