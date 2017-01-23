/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   flag_apply_spaceplus.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/12/01 15:41:39 by abombard          #+#    #+#             */
/*   Updated: 2016/12/01 15:45:13 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "printf.h"

static uint8_t	pad_one_character(t_buffer *dst, t_flags *flag, char c)
{
	if (flag->type != PRINTF_INT && flag->type != PRINTF_UINT)
		return (0);
	if (dst->data[0] == '-')
		return (0);
	if (dst->size + 1 > INTERNAL_DATA_SIZE_MAX)
	{
		dst->size = INTERNAL_DATA_SIZE_MAX;
		dst->data[INTERNAL_DATA_SIZE_MAX - 1] = '$';
		return (1);
	}
	ft_memmove(dst->data + 1, dst->data, dst->size);
	dst->data[0] = c;
	dst->size += 1;
	return (1);
}

uint8_t			flag_apply_plus(t_buffer *dst, t_flags *flag)
{
	if (!flag->plus)
		return (0);
	return (pad_one_character(dst, flag, '+'));
}

uint8_t			flag_apply_space(t_buffer *dst, t_flags *flag)
{
	if (!flag->space)
		return (0);
	return (pad_one_character(dst, flag, ' '));
}
