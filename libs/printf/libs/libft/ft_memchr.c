/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memchr.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2014/11/04 19:51:31 by abombard          #+#    #+#             */
/*   Updated: 2015/03/25 14:04:42 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_memchr(const void *s, int c, size_t n)
{
	if (s && n)
	{
		if (n == 0)
			return (NULL);
		while (*(char*)s != (char)c && --n)
			s++;
		if (*(char*)s == (char)c)
			return ((void*)s);
	}
	return (NULL);
}
