/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pop_hexa.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/12/01 15:43:43 by abombard          #+#    #+#             */
/*   Updated: 2016/12/01 15:43:44 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "printf.h"

static uint8_t	get_hex(t_buffer *dst, unsigned long long nbr)
{
	char			*base;
	char			tmp[36];
	uint32_t		x;

	if (nbr == 0)
	{
		dst->size = 0;
		return (1);
	}
	base = "0123456789abcdef";
	x = 35;
	tmp[x--] = 0;
	while (nbr > 0)
	{
		tmp[x--] = base[nbr % ft_strlen(base)];
		nbr /= ft_strlen(base);
	}
	dst->size = ft_strlen(tmp + x + 1);
	ft_memcpy(dst->data, tmp + x + 1, dst->size);
	return (1);
}

uint8_t			case_x(t_buffer *dst, va_list ap, t_flags *flag)
{
	unsigned long long	n;

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
		get_hex(dst, n);
	flag->type = PRINTF_UHEXA_MIN;
	return (1);
}

uint8_t			case_x_maj(t_buffer *dst, va_list ap, t_flags *flag)
{
	uint32_t	x;

	get_hex(dst, va_arg(ap, unsigned int));
	x = 0;
	while (x < dst->size)
	{
		dst->data[x] = ft_toupper(dst->data[x]);
		x++;
	}
	flag->type = PRINTF_UHEXA_MAJ;
	return (1);
}
