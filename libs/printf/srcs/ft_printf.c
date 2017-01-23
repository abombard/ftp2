/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_printf.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/12/01 15:42:15 by abombard          #+#    #+#             */
/*   Updated: 2016/12/01 15:42:16 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "printf.h"

int	ft_printf(const char *fmt, ...)
{
	uint32_t	ret;
	va_list		ap;
	char		*fmt_p;

	va_start(ap, fmt);
	fmt_p = (char *)fmt;
	ret = internal_printf(1, fmt_p, ap);
	va_end(ap);
	return (ret);
}
