CC=clang
FLAGS=
FLAGS42=-Wall -Wextra -Werror

NAME=client

LIBS_DIR=../libs

LIBFT_DIR=$(LIBS_DIR)/libft
PRINTF_DIR=$(LIBS_DIR)/printf
LIST_DIR=$(LIBS_DIR)/list

LIBFT=$(LIBFT_DIR)/libft.a
PRINTF=$(PRINTF_DIR)/libprintf.a
LIST=$(LIST_DIR)/liblist.a

LIBS=-L $(PRINTF_DIR) -lprintf -L $(LIBFT_DIR) -lft -L $(LIST_DIR) -llist

SRC_DIR=./
INCLUDES=-I ./includes -I $(LIBFT_DIR) -I $(LIST_DIR) -I $(PRINTF_DIR)

BUILD_DIR=__build

SRC=\
log.c\
strerror.c\
get_next_line.c\
sock.c\
client.c\

OBJ=$(addprefix $(BUILD_DIR)/,$(SRC:.c=.o))

all:$(BUILD_DIR) $(NAME)

$(BUILD_DIR):
	@mkdir -p $@

$(LIBFT):
	@make -C $(LIBFT_DIR)

$(PRINTF):
	@make -C $(PRINTF_DIR)

$(LIST):
	@make -C $(LIST_DIR)

$(BUILD_DIR)/%.o:$(SRC_DIR)/%.c
	@$(CC) $(FLAGS42) -c $< -o $@ $(INCLUDES)

$(NAME):$(LIBFT) $(PRINTF) $(LIST) $(OBJ)
	@$(CC) $(FLAGS42) $(OBJ) $(LIBS) -o $@
	@echo "$@ was created"

clean:
	@rm -rf $(BUILD_DIR)

fclean: clean
	@rm -f $(NAME)

re: fclean all

test: $(NAME)

# Make a highlight file for types.  Requires Exuberant ctags and awk
types: types.vim
types.vim: *.[ch] $(LIBS_DIR)/*/*.[h]
		ctags --c-kinds=gstu -o- *.[ch] |\
				awk 'BEGIN{printf("syntax keyword Type\t")}\
						{printf("%s ", $$1)}END{print ""}' > $@
