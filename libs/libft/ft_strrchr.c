/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strrchr.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2014/11/04 16:35:59 by abombard          #+#    #+#             */
/*   Updated: 2014/11/06 05:55:45 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char				*ft_strrchr(const char *s, int c)
{
	unsigned int	x;

	x = ft_strlen(s);
	while (s[x] != (char)c && x)
		x--;
	return (s[x] == (char)c ? (char*)(s + x) : NULL);
}
