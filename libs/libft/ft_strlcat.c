/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strlcat.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2014/11/04 23:02:42 by abombard          #+#    #+#             */
/*   Updated: 2014/11/10 16:25:38 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

size_t				ft_strlcat(char *dst, const char *src, size_t size)
{
	size_t			x;
	size_t			y;
	int				ret;

	ret = ft_strlen(dst) + ft_strlen(src);
	x = 0;
	while (dst[x] && x < size)
		x++;
	if (x == size)
		return (ft_strlen(src) + size);
	y = 0;
	while (src[y] && x < size)
		dst[x++] = src[y++];
	if (x < size)
		dst[x] = 0;
	else if (x == size)
		dst[x - 1] = 0;
	else
		dst[ret - 1] = '\0';
	return (ret);
}
