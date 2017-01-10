/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strncmp.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2014/11/03 15:03:50 by abombard          #+#    #+#             */
/*   Updated: 2016/03/31 13:19:21 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int					ft_strncmp(const char *s1, const char *s2, size_t n)
{
	size_t			x;

	if (!s1 || !s2 || !n)
		return (0);
	x = 0;
	while (s1[x] == s2[x] && x < n)
	{
		x++;
	}
	if (x == n)
		return (0);
	return (x == n ? 0 : (unsigned char)s1[x] - (unsigned char)s2[x]);
}
