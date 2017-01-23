/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_flags.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/12/01 15:42:36 by abombard          #+#    #+#             */
/*   Updated: 2016/12/01 16:19:09 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "printf.h"
#include "parse_flags.h"

static uint32_t	get_value(char *fmt, uint32_t *value)
{
	uint32_t	len;

	*value = ft_atoi(fmt);
	len = 0;
	while (ft_isdigit(fmt[len]))
		len++;
	return (len);
}

static uint32_t	get_flag(char *fmt, t_flags *flag)
{
	uint32_t	len;
	char		c;

	c = *fmt;
	len = 0;
	if (c == ' ')
		flag->space = 1;
	else if (c == '.')
		len += get_value(fmt + 1, &flag->precision) + 1;
	else if (c == '0')
		flag->zero = 1;
	else if (ft_isdigit(c))
		len += get_value(fmt, &flag->field);
	else if (c == '#')
		flag->diez = 1;
	else if (c == '+')
		flag->plus = 1;
	else if (c == '-')
		flag->moins = 1;
	else
		return (0);
	return (len == 0 ? 1 : len);
}

static uint32_t	get_modifier(char *fmt, t_modifier *modifier)
{
	static t_mod		modifier_array[] = {
		{ STR_TO_BUF("hh"), MODIFIER_HH },
		{ STR_TO_BUF("h"), MODIFIER_H },
		{ STR_TO_BUF("ll"), MODIFIER_LL },
		{ STR_TO_BUF("l"), MODIFIER_L },
		{ STR_TO_BUF("j"), MODIFIER_J },
		{ STR_TO_BUF("z"), MODIFIER_Z }
	};
	unsigned int		i;
	size_t				len;

	len = ft_strlen(fmt);
	i = 0;
	while (i < sizeof(modifier_array) / sizeof(modifier_array[0]))
	{
		if (len >= modifier_array[i].buf.size &&
!ft_memcmp(fmt, modifier_array[i].buf.data, modifier_array[i].buf.size))
		{
			*modifier = modifier_array[i].modifier;
			return (modifier_array[i].buf.size);
		}
		i++;
	}
	*modifier = MODIFIER_NONE;
	return (0);
}

extern uint32_t	flag_get(char *fmt, t_flags *flag)
{
	uint32_t	len;
	uint32_t	incr;

	len = 0;
	while ((incr = get_flag(fmt + len, flag)) > 0)
		len += incr;
	if (flag->zero && flag->moins)
		flag->zero = 0;
	len += get_modifier(fmt, &flag->modifier);
	return (len);
}
