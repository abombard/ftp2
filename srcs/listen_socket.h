#ifndef LISTEN_SOCKET_H
# define LISTEN_SOCKET_H

#include "sys_types.h"
#include "log.h"
#include "sock.h"

SOCKET	listen_socket(const char *host, const int listen_port);

#endif
