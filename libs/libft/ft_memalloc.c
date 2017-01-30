/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memalloc.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2014/11/06 18:22:20 by abombard          #+#    #+#             */
/*   Updated: 2017/01/23 14:25:42 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"
#include <stdlib.h>

void		*ft_memalloc(size_t size)
{
	char	*str;
	int		x;

	x = 0;
	if (!(str = (char*)malloc(sizeof(str) * (size + 1))))
		return (NULL);
	while (size--)
		str[x++] = 0;
	return ((void*)str);
}
