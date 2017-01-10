/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strmap.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2014/11/06 18:44:02 by abombard          #+#    #+#             */
/*   Updated: 2014/11/10 23:13:09 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char		*ft_strmap(char const *s, char (*f)(char))
{
	char	*dst;
	int		x;

	x = 0;
	dst = NULL;
	if (s && f)
	{
		if (!(dst = ft_strdup(s)))
			return (NULL);
		while (s[x])
		{
			dst[x] = f(s[x]);
			x++;
		}
	}
	return (dst);
}
