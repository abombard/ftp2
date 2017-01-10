/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memccpy.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2014/11/04 21:02:13 by abombard          #+#    #+#             */
/*   Updated: 2014/11/19 14:16:43 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void			*ft_memccpy(void *dst, const void *src, int c, size_t n)
{
	char		*srcpy;
	char		*dstpy;
	char		cpy;

	srcpy = (char*)src;
	dstpy = (char*)dst;
	cpy = (unsigned char)c;
	while (*srcpy && *srcpy != cpy && n--)
		*dstpy++ = *srcpy++;
	*dstpy = *srcpy;
	return (*srcpy == (char)cpy ? dstpy + 1 : NULL);
}
