/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memmove.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2014/11/05 17:58:16 by abombard          #+#    #+#             */
/*   Updated: 2016/12/01 16:27:22 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_memmove(void *dst, const void *src, size_t len)
{
	char s[len];

	if (!dst || !src || !len)
		return (NULL);
	ft_memcpy(s, src, len);
	ft_memcpy(dst, s, len);
	return (dst);
}
