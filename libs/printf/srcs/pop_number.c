/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pop_number.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/12/01 15:43:51 by abombard          #+#    #+#             */
/*   Updated: 2016/12/01 15:50:16 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "printf.h"
#define NB_SIZE_MAX 32

static uint8_t		ft_itoa_lg(t_buffer *dst, long long in_nb)
{
	char				buf[NB_SIZE_MAX];
	unsigned long long	nb;
	int					offset;

	offset = sizeof(buf) - 1;
	nb = in_nb < 0 ? -in_nb : in_nb;
	while (nb > 0)
	{
		if (offset < 0)
			return (0);
		buf[offset] = nb % 10 + '0';
		offset--;
		nb = nb / 10;
	}
	if (in_nb < 0)
	{
		buf[offset] = '-';
		offset--;
	}
	dst->size = sizeof(buf) - offset - 1;
	ft_memcpy(dst->data, buf + offset + 1, dst->size);
	return (1);
}

uint8_t				case_nbr(t_buffer *dst, va_list ap, t_flags *flag)
{
	long long		n;

	if (flag->modifier == MODIFIER_J)
		n = va_arg(ap, intmax_t);
	else if (flag->modifier == MODIFIER_Z)
		n = va_arg(ap, size_t);
	else if (flag->modifier == MODIFIER_H)
		n = va_arg(ap, int);
	else if (flag->modifier == MODIFIER_HH)
		n = va_arg(ap, int);
	else if (flag->modifier == MODIFIER_L)
		n = va_arg(ap, long);
	else if (flag->modifier == MODIFIER_LL)
		n = va_arg(ap, long long);
	else
		n = va_arg(ap, int);
	if (n == 0)
	{
		dst->data[0] = '0';
		dst->size = 1;
	}
	else
		ft_itoa_lg(dst, n);
	flag->type = PRINTF_INT;
	return (1);
}
