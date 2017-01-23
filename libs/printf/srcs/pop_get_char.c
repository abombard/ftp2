/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pop_get_char.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/12/01 15:43:04 by abombard          #+#    #+#             */
/*   Updated: 2016/12/01 15:43:05 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "printf.h"

static uint32_t	bin_size(wchar_t n)
{
	long	nb;
	int		ret;

	ret = 0;
	nb = n;
	if (nb < 0)
		nb = -nb;
	while (nb >= 2)
	{
		nb /= 2;
		ret += 1;
	}
	return (ret);
}

static uint8_t	get_wchar(t_buffer *dst, wchar_t ch, int size)
{
	if (size < 12)
	{
		dst->data[dst->size + 0] = (6 << 5) | (((ch >> 6) << 27) >> 27);
		dst->data[dst->size + 1] = (2 << 6) | ((ch << 26) >> 26);
		dst->data[dst->size + 2] = 0;
		dst->size += 2;
	}
	else if (size < 17)
	{
		dst->data[dst->size + 0] = (14 << 4) | (((ch >> 12) << 28) >> 28);
		dst->data[dst->size + 1] = (2 << 6) | (((ch >> 6) << 26) >> 26);
		dst->data[dst->size + 2] = (2 << 6) | ((ch << 26) >> 26);
		dst->data[dst->size + 3] = 0;
		dst->size += 3;
	}
	else
	{
		dst->data[dst->size + 0] = (30 << 3) | (((ch >> 18) << 29) >> 29);
		dst->data[dst->size + 1] = (2 << 6) | (((ch >> 12) << 26) >> 26);
		dst->data[dst->size + 2] = (2 << 6) | (((ch >> 6) << 26) >> 26);
		dst->data[dst->size + 3] = (2 << 6) | ((ch << 26) >> 26);
		dst->data[dst->size + 4] = 0;
		dst->size += 4;
	}
	return (1);
}

uint8_t			get_char(t_buffer *dst, wchar_t ch)
{
	uint8_t		size;

	size = bin_size(ch);
	if (size < 8)
	{
		dst->data[dst->size] = (unsigned char)ch;
		dst->data[dst->size + 1] = 0;
		dst->size += 1;
	}
	else
		get_wchar(dst, ch, size);
	return (1);
}
