/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   printf.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abombard <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/04/19 12:45:28 by abombard          #+#    #+#             */
/*   Updated: 2016/12/01 17:43:03 by abombard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_PRINTF_H
# define FT_PRINTF_H

# include <stdint.h>
# include <stdarg.h>
# include <unistd.h>
# include "libft.h"
# include <stdio.h>

# define PRINTF_BUFFER_SIZE_MAX					2048
# define INTERNAL_DATA_SIZE_MAX					512

typedef struct	s_buffer
{
	uint8_t			*data;
	uint32_t		size;
}				t_buffer;

typedef enum	e_modifier
{
	MODIFIER_NONE,
	MODIFIER_H,
	MODIFIER_HH,
	MODIFIER_L,
	MODIFIER_LL,
	MODIFIER_J,
	MODIFIER_Z
}				t_modifier;

typedef enum	e_var_type
{
	PRINTF_CHAR,
	PRINTF_STRING,
	PRINTF_INT,
	PRINTF_UINT,
	PRINTF_UOCTAL,
	PRINTF_UHEXA_MIN,
	PRINTF_UHEXA_MAJ,
	PRINTF_POINTER
}				t_var_type;

typedef struct	s_flags
{
	uint8_t			space;
	uint32_t		precision;
	uint32_t		field;
	uint8_t			zero;
	uint8_t			diez;
	uint8_t			plus;
	uint8_t			moins;
	t_modifier		modifier;
	t_var_type		type;
}				t_flags;
typedef struct	s_norm
{
	uint32_t		str_index;
	uint32_t		fmt_len;
}				t_norm;

uint8_t			case_nbr(t_buffer *dst, va_list ap, t_flags *flag);
uint8_t			case_p(t_buffer *dst, va_list ap, t_flags *flag);
uint8_t			case_x(t_buffer	*dst, va_list ap, t_flags *flag);
uint8_t			case_x_maj(t_buffer	*dst, va_list ap, t_flags *flag);
uint8_t			case_o(t_buffer *dst, va_list ap, t_flags *flag);
uint8_t			case_u(t_buffer *dst, va_list ap, t_flags *flag);
uint8_t			case_c(t_buffer *dst, va_list ap, t_flags *flag);
uint8_t			case_s(t_buffer *dst, va_list ap, t_flags *flag);
uint8_t			get_char(t_buffer *dst, wchar_t ch);
uint32_t		flag_get(char *fmt, t_flags *flag);
uint8_t			flag_apply_main(t_buffer *dst, t_flags *flag);
uint8_t			flag_apply_field_space(t_buffer *dst, t_flags *flags);
uint8_t			flag_apply_field_zero(t_buffer *dst, t_flags *flags);
uint8_t			flag_apply_field_precision(t_buffer *dst, t_flags *flags);
uint8_t			flag_apply_space(t_buffer *dst, t_flags *flag);
uint8_t			flag_apply_plus(t_buffer *dst, t_flags *flag);
uint8_t			flag_apply_diez(t_buffer *dst, uint32_t type);
int				internal_printf(int fd, char *fmt, va_list ap);
uint8_t			field_right(t_buffer *dst, int len, uint8_t c);

#endif
