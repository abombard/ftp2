/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strsplit.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/03/31 13:21:34 by abombard          #+#    #+#             */
/*   Updated: 2016/03/31 13:23:11 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	nb_arg(char *s, char c)
{
	int	ret;
	int	i;

	ret = 0;
	i = 0;
	while (*s)
	{
		if (*s == c && i)
		{
			ret += 1;
			i = 0;
		}
		else
			++i;
		++s;
	}
	return (i ? ret + 1 : ret);
}

static char	*get_str(char **s, char c)
{
	char		buf[ft_strlen(*s) + 1];
	int			i;
	char		*pt;

	pt = *s;
	i = 0;
	while (*pt && *pt != c)
	{
		buf[i++] = *pt++;
	}
	buf[i] = 0;
	*s = pt;
	return (ft_strdup(buf));
}

static char	**split_but(char *s, char c, int len)
{
	char		**ret;
	int			i;

	ret = malloc(sizeof(char *) * (len + 1));
	if (!ret)
		return (NULL);
	i = 0;
	while (i < len)
	{
		ret[i++] = get_str(&s, c);
		while (*s == c)
			s++;
		if (!*s)
			break ;
	}
	ret[i] = 0;
	return (ret);
}

char		**ft_strsplit(char *s, char c)
{
	int	len;

	if (!s)
		return (NULL);
	while (*s && *s == c)
		s++;
	if (!*s)
		return (NULL);
	len = nb_arg(s, c);
	if (!len)
		return (NULL);
	return (split_but(s, c, len));
}
