/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pop_char.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/12/01 15:42:45 by abombard          #+#    #+#             */
/*   Updated: 2016/12/01 15:45:35 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "printf.h"
#include <wchar.h>

static void		case_s_wchar(t_buffer *dst, va_list ap)
{
	wchar_t	*wc;
	int		x;

	wc = va_arg(ap, wchar_t *);
	x = 0;
	while (wc[x])
	{
		get_char(dst, wc[x]);
		x++;
	}
}

static void		internal_case_s(t_buffer *dst, va_list ap)
{
	char	*s;
	size_t	len;

	s = va_arg(ap, char *);
	if (s == NULL)
	{
		dst->size = sizeof("(null)") - 1;
		ft_memcpy(dst->data, "(null)", dst->size);
	}
	else
	{
		len = ft_strlen(s);
		if (len > INTERNAL_DATA_SIZE_MAX)
		{
			len = INTERNAL_DATA_SIZE_MAX - 1;
			dst->data[INTERNAL_DATA_SIZE_MAX - 1] = '$';
			dst->size = INTERNAL_DATA_SIZE_MAX;
		}
		else
			dst->size = len;
		ft_memcpy(dst->data, s, len);
	}
}

uint8_t			case_s(t_buffer *dst, va_list ap, t_flags *flag)
{
	if (flag->modifier == MODIFIER_L)
		case_s_wchar(dst, ap);
	else
		internal_case_s(dst, ap);
	flag->type = PRINTF_STRING;
	return (1);
}

uint8_t			case_c(t_buffer *dst, va_list ap, t_flags *flag)
{
	unsigned int	c;

	c = va_arg(ap, unsigned int);
	get_char(dst, c);
	flag->type = PRINTF_CHAR;
	return (1);
}
