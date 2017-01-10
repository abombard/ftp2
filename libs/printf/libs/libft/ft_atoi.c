/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_atoi.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/03/30 17:15:38 by abombard          #+#    #+#             */
/*   Updated: 2016/12/01 16:25:03 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int			ft_atoi(const char *str)
{
	int	nbr;
	int	is_negativ;
	int	i;

	i = 0;
	is_negativ = 0;
	if (str[0] == '-')
	{
		is_negativ = 1;
		i++;
	}
	nbr = 0;
	while (ft_isdigit(str[i]))
	{
		nbr = (nbr * 10) + str[i] - '0';
		i++;
	}
	return (is_negativ ? -nbr : nbr);
}

long long	ft_atol(const char *str)
{
	long long	nbr;
	int			is_negativ;
	int			i;

	i = 0;
	is_negativ = 0;
	if (str[0] == '-')
	{
		is_negativ = 1;
		i++;
	}
	nbr = 0;
	while (ft_isdigit(str[i]))
	{
		nbr = (nbr * 10) + str[i] - '0';
		i++;
	}
	return (is_negativ ? -nbr : nbr);
}
