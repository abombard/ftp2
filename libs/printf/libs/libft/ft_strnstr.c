/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strnstr.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2014/11/03 17:24:11 by abombard          #+#    #+#             */
/*   Updated: 2014/11/08 09:14:35 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char				*ft_strnstr(const char *s1, const char *s2, size_t n)
{
	size_t			x;

	x = 0;
	if (s2[0] == '\0')
		return ((char*)s1);
	if (*s1 == '\0' || n == 0)
		return (NULL);
	if (*s1 != *s2 && *s1)
		return (ft_strnstr(s1 + 1, s2, n - 1));
	while (s1[x] == s2[x] && s1[x] && s2[x] && x < n)
		x = x + 1;
	if (s2[x] == '\0')
		return ((char*)s1);
	return (ft_strnstr(s1 + 1, s2, n - 1));
}
