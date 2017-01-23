/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pop_octal.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/12/01 15:43:58 by abombard          #+#    #+#             */
/*   Updated: 2016/12/01 15:43:59 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "printf.h"

static uint8_t	get_nbr_oct(t_buffer *dst, unsigned long nbr)
{
	char	*base;
	char	tmp[36];
	int		x;

	if (nbr == 0)
	{
		dst->size = 0;
		return (1);
	}
	base = "01234567";
	x = 35;
	tmp[x--] = 0;
	while (nbr > 0)
	{
		tmp[x--] = base[nbr % ft_strlen(base)];
		nbr /= ft_strlen(base);
	}
	ft_strcpy((char *)dst->data, tmp + x + 1);
	dst->size = ft_strlen((char *)dst->data);
	return (1);
}

uint8_t			case_o(t_buffer *dst, va_list ap, t_flags *flag)
{
	unsigned long long		n;

	if (flag->modifier == MODIFIER_H)
		n = (unsigned short)va_arg(ap, int);
	else if (flag->modifier == MODIFIER_HH)
		n = (unsigned char)va_arg(ap, int);
	else if (flag->modifier == MODIFIER_L)
		n = (unsigned long)va_arg(ap, unsigned long);
	else if (flag->modifier == MODIFIER_LL)
		n = va_arg(ap, unsigned long long);
	else
		n = (unsigned int)va_arg(ap, int);
	if (n == 0)
	{
		dst->data[0] = '0';
		dst->size = 1;
	}
	else
		get_nbr_oct(dst, n);
	flag->type = PRINTF_UOCTAL;
	return (1);
}
