/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pop_pointer.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/12/01 15:44:06 by abombard          #+#    #+#             */
/*   Updated: 2016/12/01 15:44:07 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "printf.h"

static void			base_init(char *base)
{
	base[0] = '0';
	base[1] = '1';
	base[2] = '2';
	base[3] = '3';
	base[4] = '4';
	base[5] = '5';
	base[6] = '6';
	base[7] = '7';
	base[8] = '8';
	base[9] = '9';
	base[10] = 'a';
	base[11] = 'b';
	base[12] = 'c';
	base[13] = 'd';
	base[14] = 'e';
	base[15] = 'f';
}

static uint8_t		get_nbr_hex(t_buffer *dst, unsigned long long nbr)
{
	char				base[16];
	int					x;

	base_init(base);
	x = 14;
	while (nbr > 0)
	{
		x--;
		if (x < 0)
			return (0);
		dst->data[x] = base[nbr % sizeof(base)];
		nbr /= sizeof(base);
	}
	if (x > 0)
		ft_memcpy(dst->data, dst->data + x, 14 - x);
	dst->size = 14 - x;
	return (1);
}

uint8_t				case_p(t_buffer *dst, va_list ap, t_flags *flag)
{
	void	*pt;

	pt = va_arg(ap, void *);
	if (pt == NULL)
	{
		dst->size = sizeof("(nil)");
		ft_memcpy(dst->data, "(nil)", dst->size);
	}
	else if (!get_nbr_hex(dst, (unsigned long long)pt))
		return (0);
	flag->diez = 1;
	flag->type = PRINTF_POINTER;
	return (1);
}
