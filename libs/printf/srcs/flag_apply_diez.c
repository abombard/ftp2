/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   flag_apply_diez.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/12/01 15:41:09 by abombard          #+#    #+#             */
/*   Updated: 2016/12/01 15:44:53 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "printf.h"

uint8_t		flag_apply_diez(t_buffer *dst, t_var_type type)
{
	char	*diez;
	int		diez_size;

	if (!ft_memcmp(dst->data, "(nil)", 4) || !ft_memcmp(dst->data, "(Null)", 6)
			|| (dst->size == 1 && !ft_memcmp(dst->data, "0", 1)))
		return (0);
	diez = "";
	if (type == PRINTF_UOCTAL)
		diez = "0";
	else if (type == PRINTF_UHEXA_MIN || type == PRINTF_POINTER ||
			type == PRINTF_POINTER)
		diez = "0x";
	else if (type == PRINTF_UHEXA_MAJ)
		diez = "0X";
	diez_size = ft_strlen(diez);
	if (dst->size + diez_size > INTERNAL_DATA_SIZE_MAX)
		return (0);
	ft_memmove(dst->data + diez_size, dst->data, dst->size);
	ft_memcpy(dst->data, diez, diez_size);
	dst->size += diez_size;
	return (1);
}
