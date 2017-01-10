/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_itoa.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2014/11/06 22:23:10 by abombard          #+#    #+#             */
/*   Updated: 2015/02/04 21:43:53 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static char		*make_nbr(char *nbr, long long n, int len)
{
	unsigned long long	i;

	if (n < 0)
	{
		nbr[0] = '-';
		i = -n;
	}
	else
		i = n;
	if (i == 0)
		nbr[0] = '0';
	while (i > 0)
	{
		nbr[len - 1] = i % 10 + '0';
		i = i / 10;
		len--;
	}
	return (nbr);
}

char			*ft_itoa(int n)
{
	char		*nbr;
	long long	save;
	int			len;

	len = 0;
	save = (long long)n;
	if (save < 0)
	{
		len++;
		save = save * -1;
	}
	while (save > 0)
	{
		save = save / 10;
		len++;
	}
	if (!(nbr = (char*)ft_memalloc(len + 1)))
		return (NULL);
	nbr[len + 1] = '\0';
	nbr = make_nbr(nbr, (long long)n, len);
	return (nbr);
}

char			*ft_itoa_lg(long long n)
{
	char				*nbr;
	unsigned long long	save;
	int					len;

	len = 0;
	if (n < 0)
	{
		len++;
		save = -n;
	}
	else
		save = n;
	while (save > 0)
	{
		save = save / 10;
		len++;
	}
	if (!(nbr = (char*)ft_memalloc(len + 1)))
		return (NULL);
	nbr[len + 1] = '\0';
	nbr = make_nbr(nbr, n, len);
	return (nbr);
}

static char		*make_nbr_ull(char *nbr, unsigned long long n, int len)
{
	if (n == 0)
		nbr[0] = '0';
	while (n > 0)
	{
		nbr[len - 1] = n % 10 + '0';
		n = n / 10;
		len--;
	}
	return (nbr);
}

char			*ft_itoa_ull(unsigned long long n)
{
	char				*nbr;
	unsigned long long	save;
	int					len;

	len = 0;
	save = n;
	while (save > 0)
	{
		save = save / 10;
		len++;
	}
	if (!(nbr = (char*)ft_memalloc(len + 1)))
		return (NULL);
	nbr[len + 1] = '\0';
	nbr = make_nbr_ull(nbr, n, len);
	return (nbr);
}
