#ifndef SOCK_H
# define SOCK_H

#include "sys_types.h"
#include "log.h"

#ifdef WIN32 /* si vous êtes sous Windows */

#include <winsock2.h>

#elif defined (linux) /* si vous êtes sous Linux */

#include <sys/types.h>
#include <sys/socket.h>		/* socket(), AF_INET, SOCK_STREAM */
#include <netinet/in.h>
#include <arpa/inet.h>		/* htons */
#include <netdb.h>			/* gethostbyname */
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

#else /* sinon vous êtes sur une plateforme non supportée */

# error not defined for this platform

#endif

SOCKET socket_open (const int ai_family,
				const int ai_socktype,
				const int ai_protocol);

void socket_close (SOCKET sock);

#endif
