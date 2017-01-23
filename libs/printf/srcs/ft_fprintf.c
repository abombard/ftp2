/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_fprintf.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/12/01 15:41:50 by abombard          #+#    #+#             */
/*   Updated: 2016/12/01 15:41:51 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "printf.h"

int		ft_fprintf(int fd, const char *fmt, ...)
{
	uint32_t		ret;
	va_list			ap;
	char			*fmt_p;

	va_start(ap, fmt);
	fmt_p = (char *)fmt;
	ret = internal_printf(fd, fmt_p, ap);
	va_end(ap);
	return (ret);
}
