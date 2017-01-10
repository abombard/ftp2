/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strjoin.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2014/11/06 22:04:28 by abombard          #+#    #+#             */
/*   Updated: 2015/03/25 14:05:27 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char		*ft_strjoin(char const *s1, char const *s2)
{
	char	*dst;

	dst = NULL;
	if (s1 && s2)
	{
		if (!(dst = (char*)ft_memalloc(ft_strlen(s1) + ft_strlen(s2))))
			return (NULL);
		ft_strcpy(dst, (char*)s1);
		ft_strcat(dst, (char*)s2);
	}
	return (dst);
}
