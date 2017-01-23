/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   internal_printf.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/12/01 15:42:28 by abombard          #+#    #+#             */
/*   Updated: 2016/12/01 15:53:33 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "printf.h"

static uint8_t	switch_case(t_buffer *dst, char c, va_list ap, t_flags *flag)
{
	if (c == 'd' || c == 'i')
		case_nbr(dst, ap, flag);
	else if (c == 'o')
		case_o(dst, ap, flag);
	else if (c == 'u')
		case_u(dst, ap, flag);
	else if (c == 'x')
		case_x(dst, ap, flag);
	else if (c == 'X')
		case_x_maj(dst, ap, flag);
	else if (c == 'p')
		case_p(dst, ap, flag);
	else if (c == 's')
		case_s(dst, ap, flag);
	else if (c == 'c')
		case_c(dst, ap, flag);
	else
		return (0);
	return (1);
}

static uint32_t	get_argument_on_stack(t_buffer *dst, char *fmt, va_list ap)
{
	uint32_t		len;
	t_flags			flag;
	char			c;

	dst->size = 0;
	ft_memset(&flag, 0, sizeof(flag));
	len = flag_get(fmt, &flag);
	c = fmt[len];
	if ('A' <= c && c <= 'Z' && c != 'X')
	{
		c += 32;
		flag.modifier = MODIFIER_L;
	}
	if (!switch_case(dst, c, ap, &flag))
	{
		ft_memcpy(dst->data, fmt, len + 1);
		dst->size = len + 1;
		return (len + 1);
	}
	len += 1;
	flag_apply_main(dst, &flag);
	return (len);
}

static uint32_t	get_next_part(t_buffer *dst, char *fmt, va_list ap)
{
	uint32_t	len;

	if (*fmt == '%')
	{
		len = get_argument_on_stack(dst, fmt + 1, ap);
		if (len == 0)
			return (0);
		len += 1;
	}
	else
	{
		len = 0;
		while (fmt[len] && fmt[len] != '%')
		{
			if (len == INTERNAL_DATA_SIZE_MAX)
			{
				dst->data[len - 1] = '$';
				break ;
			}
			dst->data[len] = fmt[len];
			len++;
		}
		dst->size = len;
	}
	return (len);
}

static uint32_t	dump_buf(int fd, char *buf, uint32_t *size)
{
	uint32_t	count;

	count = write(fd, buf, *size);
	*size = 0;
	return (count);
}

int				internal_printf(int fd, char *fmt, va_list ap)
{
	char			str[PRINTF_BUFFER_SIZE_MAX];
	t_norm			n;
	t_buffer		buf;
	uint8_t			bytes[INTERNAL_DATA_SIZE_MAX];
	uint32_t		count;

	count = 0;
	n.str_index = 0;
	while (*fmt != 0)
	{
		buf.data = bytes;
		n.fmt_len = get_next_part(&buf, fmt, ap);
		if (n.fmt_len == 0)
			return (-1);
		fmt += n.fmt_len;
		if (n.str_index + buf.size > sizeof(str))
			count += dump_buf(fd, str, &n.str_index);
		ft_memcpy(str + n.str_index, buf.data, buf.size);
		n.str_index += buf.size;
	}
	if (n.str_index)
		count += dump_buf(fd, str, &n.str_index);
	return (count);
}
