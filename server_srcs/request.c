#include "server.h"
#include "strerror.h"
#include "libft.h"
#include "request.h"
#include <stdlib.h>

static int	match_request(int argc, char **argv, t_user *user, t_io *io)
{
	static t_request	requests[] = {
		{ "PWD", request_pwd },
		{ "USER", request_user },
		{ "QUIT", request_quit },
		{ "SYST", request_syst },
		{ "LS", request_ls },
		{ "CD", request_cd },
		{ "GET", request_get },
		{ "PUT", request_put }
	};
	size_t				i;

	i = 0;
	while (i < sizeof(requests) / sizeof(requests[0]))
	{
		if (!ft_strcmp(argv[0], requests[i].str))
			return (requests[i].func(argc, argv, user, io));
		i++;
	}
	return (EBADRQC);
}

static bool	treat_request(int ac, char **av, t_user *user, t_io *io)
{
	t_data	*data;
	char	*err;
	int		errnum;

	errnum = match_request(ac, av, user, io);
	if (errnum)
	{
		err = strerror(errnum);
		if (!err)
			err = "Undefined error";
		data = msg_error(err);
		if (!data)
			return (false);
		list_add_tail(&data->list, &io->datas_out);
	}
	return (true);
}

static bool	get_request(t_data *in, t_buf *request)
{
	char	*pt;

	pt = ft_memchr(in->bytes, '\n', in->size);
	if (pt == NULL)
		return (false);
	request->bytes = in->bytes;
	request->size = (size_t)(pt - in->bytes);
	return (true);
}

static bool	split_request(t_buf *request, int *argc, char ***argv)
{
	size_t	i;

	request->bytes[request->size] = '\0';
	*argv = strsplit_whitespace(request->bytes);
	request->bytes[request->size] = '\n';
	if (!*argv)
		return (false);
	*argc = 0;
	while ((*argv)[*argc])
		(*argc) += 1;
	i = 0;
	while ((*argv)[0][i])
	{
		(*argv)[0][i] = (char)ft_toupper((*argv)[0][i]);
		i++;
	}
	return (true);
}

extern bool	request(t_server *server, t_io *io)
{
	int			argc;
	char		**argv;
	t_user		*user;
	t_buf		request;
	bool		status;

	if (io->data_in.fd != -1)
		return (true);
	if (!get_request(&io->data_in, &request))
		return (true);
	split_request(&request, &argc, &argv);
	request.size += 1;
	ft_memmove(io->data_in.bytes, io->data_in.bytes + request.size,
			io->data_in.size - request.size);
	io->data_in.size -= request.size;
	if (!argv)
		return (false);
	user = get_user(io->sock, server);
	status = treat_request(argc, argv, user, io);
	argc = 0;
	while (argv[argc])
		free(argv[argc++]);
	free(argv);
	return (status);
}
