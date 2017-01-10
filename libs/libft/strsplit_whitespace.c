/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   strsplit_whitespace.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/03/31 13:20:29 by abombard          #+#    #+#             */
/*   Updated: 2016/03/31 13:22:42 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	nb_arg(char *s)
{
	int	ret;
	int	i;

	if (!s)
		return (0);
	ret = 0;
	i = 0;
	while (*s)
	{
		if (ft_isspace(*s) && i)
		{
			ret += 1;
			i = 0;
		}
		else
			i++;
		s++;
	}
	return (i ? ret + 1 : ret);
}

static char	*get_str(char **s)
{
	char		buf[ft_strlen(*s) + 1];
	int			i;
	char		*pt;

	pt = *s;
	i = 0;
	while (*pt && !ft_isspace(*pt))
	{
		buf[i++] = *pt++;
	}
	buf[i] = 0;
	*s = pt;
	return (ft_strdup(buf));
}

static char	**split_but(char *s, int len)
{
	char		**ret;
	int			i;

	ret = malloc(sizeof(char *) * (len + 1));
	if (!ret)
		return (NULL);
	i = 0;
	while (1)
	{
		ret[i++] = get_str(&s);
		while (ft_isspace(*s))
			s++;
		if (!*s)
			break ;
	}
	ret[i] = 0;
	return (ret);
}

char		**strsplit_whitespace(char *s)
{
	int	len;

	if (!s)
		return (NULL);
	while (*s && ft_isspace(*s))
		s++;
	len = nb_arg(s);
	if (!len)
		return (NULL);
	return (split_but(s, len));
}
