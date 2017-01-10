/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strmapi.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2014/11/06 20:29:25 by abombard          #+#    #+#             */
/*   Updated: 2014/11/10 23:13:33 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char		*ft_strmapi(char const *s, char (*f)(unsigned int, char))
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
			dst[x] = f(x, s[x]);
			x++;
		}
	}
	return (dst);
}
