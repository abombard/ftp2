CC=gcc
FLAGS=-Wall -Wextra -Werror
PRINTF=libprintf.a
SRC_DIR=srcs
LIBFT_DIR=libs/libft
INCLUDES=-I includes
BUILD_DIR= __build
LIBS=-L $(LIBFT_DIR) -lft

SRC=ft_fprintf.c\
	ft_printf.c\
	internal_printf.c\
	parse_flags.c\
	flag_apply_main.c\
	flag_apply_field.c\
	flag_apply_field2.c\
	flag_apply_spaceplus.c\
	flag_apply_diez.c\
	pop_number.c\
	pop_pointer.c\
	pop_octal.c\
	pop_hexa.c\
	pop_decimal.c\
	pop_char.c\
	pop_get_char.c

OBJ=$(addprefix $(BUILD_DIR)/,$(SRC:.c=.o))

all:$(BUILD_DIR) $(PRINTF)

$(BUILD_DIR):
	@mkdir -p $@

$(BUILD_DIR)/%.o:$(SRC_DIR)/%.c
	@$(CC) $(FLAGS) -c $< -o $@ $(INCLUDES)

$(PRINTF):$(OBJ)
	@ar rc $@ $(OBJ)
	@ranlib $@
	@echo "$@ was created"

test: $(PRINTF)
	@make -C $(LIBFT_DIR)
	@$(CC) test.c -L ./ -lprintf -o $@ $(LIBS) $(INCLUDES)
	@echo "$@ was created"

clean:
	@rm -rf $(BUILD_DIR)

fclean: clean
	@make $@ -C $(LIBFT_DIR)
	@rm -f $(PRINTF)
	@rm -f test

re: fclean all
