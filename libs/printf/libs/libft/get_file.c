/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_file.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/03/31 13:19:51 by abombard          #+#    #+#             */
/*   Updated: 2016/03/31 13:20:05 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"
#include <fcntl.h>

char	*get_file(char *arg)
{
	int			fd;
	int			len;
	char		*file;

	fd = open(arg, O_RDONLY);
	if (fd == -1)
		return (NULL);
	len = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	file = malloc(sizeof(char) * (len + 1));
	if (!file)
		return (NULL);
	if (read(fd, file, len) == -1)
		return (NULL);
	file[len] = 0;
	close(fd);
	return (file);
}
