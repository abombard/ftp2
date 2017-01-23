/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   flag_apply_field.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/12/01 15:41:19 by abombard          #+#    #+#             */
/*   Updated: 2016/12/01 16:37:37 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "printf.h"

static uint8_t	field_left(t_buffer *dst, int len, uint8_t c)
{
	if (len < 0)
		return (0);
	if (dst->size + len > INTERNAL_DATA_SIZE_MAX)
	{
		dst->data[dst->size - 1] = '$';
		return (0);
	}
	ft_memmove(dst->data + len, dst->data, dst->size);
	ft_memset(dst->data, c, len);
	dst->size += len;
	return (1);
}

uint8_t			flag_apply_field(t_buffer *dst, t_flags *flag, char c)
{
	int			len;

	len = flag->field - dst->size;
	if (flag->space)
		len--;
	if (flag->moins)
		field_right(dst, len, c);
	else
		field_left(dst, len, c);
	return (1);
}

uint8_t			flag_apply_field_zero(t_buffer *dst, t_flags *flag)
{
	if (!flag->zero)
		return (0);
	if ((flag->type == PRINTF_UHEXA_MAJ || flag->type == PRINTF_UHEXA_MIN ||
				flag->type == PRINTF_POINTER) && flag->diez)
		flag->field -= 2;
	return (flag_apply_field(dst, flag, '0'));
}

uint8_t			flag_apply_field_space(t_buffer *dst, t_flags *flag)
{
	if (!flag->field)
		return (0);
	return (flag_apply_field(dst, flag, ' '));
}

uint8_t			flag_apply_field_precision(t_buffer *dst, t_flags *flag)
{
	if (!flag->precision)
		return (0);
	if (flag->precision > dst->size)
	{
		if (flag->type != PRINTF_CHAR && flag->type != PRINTF_STRING)
			return (field_left(dst, flag->precision - dst->size, '0'));
	}
	else
		dst->size = flag->precision;
	return (1);
}
