/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strstr.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2014/11/03 15:04:36 by abombard          #+#    #+#             */
/*   Updated: 2014/11/04 14:01:31 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strstr(const char *s1, const char *s2)
{
	int	x;

	x = 0;
	if (s2[0] == '\0')
		return ((char*)s1);
	if (*s1 == '\0')
		return (NULL);
	if (*s1 != *s2 && *s1)
		return (ft_strstr(s1 + 1, s2));
	while (s1[x] == s2[x] && s1[x] && s2[x])
		x = x + 1;
	if (s2[x] == '\0')
		return ((char*)s1);
	return (ft_strstr(s1 + 1, s2));
}
