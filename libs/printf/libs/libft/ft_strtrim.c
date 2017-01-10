/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strtrim.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2014/11/06 22:43:27 by abombard          #+#    #+#             */
/*   Updated: 2014/11/10 17:11:41 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char		*ft_strtrim(char const *s)
{
	char	*dst;
	int		len;

	if (!s)
		return (NULL);
	dst = NULL;
	while ((*s == ' ' || *s == '\n' || *s == '\t') && *s)
		s++;
	len = ft_strlen(s) - 1;
	while ((s[len] == ' ' || s[len] == '\n' || s[len] == '\t') && len >= 0)
		len--;
	len++;
	if ((dst = (char*)ft_memalloc(len + 1)))
	{
		if (len == 1 && ft_strlen(s) != 1 && !(*s))
			dst[0] = 0;
		else
			ft_strncpy(dst, s, len);
	}
	return (dst);
}
