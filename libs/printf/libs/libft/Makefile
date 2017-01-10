CC=gcc
FLAGS=-Wall -Wextra -Werror
NAME=libft.a
SRC_DIR=srcs
BUILD_DIR= __build

SRC=ft_putchar.c\
		ft_putchar_fd.c\
		ft_putstr.c\
		ft_putstr_fd.c\
		ft_putendl.c\
		ft_putendl_fd.c\
		ft_putnbr.c\
		ft_putnbr_fd.c\
		ft_strlen.c\
		ft_strcpy.c\
		ft_strncpy.c\
		ft_strcmp.c\
		ft_strncmp.c\
		ft_tolower.c\
		ft_toupper.c\
		ft_strcapitalize.c\
		ft_isdigit.c\
		ft_isalpha.c\
		ft_isalnum.c\
		ft_isprint.c\
		ft_isascii.c\
		ft_isspace.c\
		ft_strdup.c\
		ft_strcat.c\
		ft_strncat.c\
		ft_strlcat.c\
		ft_strstr.c\
		ft_strnstr.c\
		ft_strchr.c\
		ft_strrchr.c\
		ft_memchr.c\
		ft_memcmp.c\
		ft_memcpy.c\
		ft_memccpy.c\
		ft_memmove.c\
		ft_memset.c\
		ft_bzero.c\
		ft_memalloc.c\
		ft_memdel.c\
		ft_strnew.c\
		ft_strdel.c\
		ft_strclr.c\
		ft_striter.c\
		ft_striteri.c\
		ft_strmap.c\
		ft_strmapi.c\
		ft_strequ.c\
		ft_strnequ.c\
		ft_strsub.c\
		ft_strjoin.c\
		ft_strtrim.c\
		ft_atoi.c\
		ft_itoa.c\
		ft_strsplit.c\
		get_file.c\


OBJ=$(addprefix $(BUILD_DIR)/,$(SRC:.c=.o))

all:$(BUILD_DIR) $(NAME)

$(BUILD_DIR):
	@mkdir -p $@

$(BUILD_DIR)/%.o:%.c
	@$(CC) $(FLAGS) -c $< -o $@

$(NAME):$(OBJ)
	@ar rc $(NAME) $(OBJ)
	@ranlib $(NAME)
	@echo "$@ was created"

clean:
	@rm -rf $(BUILD_DIR)

fclean: clean
	@rm -f $(NAME)

re: fclean all
