/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pop_decimal.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/12/01 15:42:52 by abombard          #+#    #+#             */
/*   Updated: 2016/12/01 15:42:53 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "printf.h"

static uint8_t	ft_itoa_ull(t_buffer *dst, unsigned long long n)
{
	unsigned long long	old_n;
	uint32_t			len;

	len = 0;
	old_n = n;
	while (n > 0)
	{
		n /= 10;
		len++;
	}
	dst->size = len;
	n = old_n;
	while (n > 0)
	{
		dst->data[len - 1] = n % 10 + '0';
		n = n / 10;
		len--;
	}
	return (1);
}

uint8_t			case_u(t_buffer *dst, va_list ap, t_flags *flag)
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
		ft_itoa_ull(dst, n);
	flag->type = PRINTF_UINT;
	return (1);
}
