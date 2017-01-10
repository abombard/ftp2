CC=clang
FLAGS=
FLAGS42=-Wall -Wextra -Werror -Wconversion

NAME=server

LIBS_DIR=./libs
DIR_LIBFT=$(LIBS_DIR)/libft
DIR_PRINTF=$(LIBS_DIR)/printf
DIR_LIST=$(LIBS_DIR)/list

LIBS=-L $(DIR_PRINTF) -lprintf -L $(DIR_LIBFT) -lft -L $(DIR_LIST) -llist

SRC_DIR=srcs
INCLUDES=-I ./includes -I $(DIR_LIBFT) -I $(DIR_LIST) -I $(DIR_PRINTF)

BUILD_DIR=__build

SRC=\
server.c\
listen_socket.c\
log.c\
sock.c\
strerror.c\

OBJ=$(addprefix $(BUILD_DIR)/,$(SRC:.c=.o))

all:$(BUILD_DIR) $(NAME)

$(BUILD_DIR):
	@mkdir -p $@

exec:
	@make -C $(DIR_LIBFT)
	@make -C $(DIR_PRINTF)
	@make -C $(DIR_LIST)

$(BUILD_DIR)/%.o:$(SRC_DIR)/%.c
	@$(CC) $(FLAGS42) -c $< -o $@ $(INCLUDES)

$(NAME): exec $(OBJ)
	@$(CC) $(FLAGS) $(OBJ) $(LIBS) -o $@
	@echo "$@ was created"

clean:
	@rm -rf $(BUILD_DIR)

fclean: clean
	@rm -f $(NAME)
	@make $@ -C $(DIR_LIBFT)
	@make $@ -C $(DIR_PRINTF)
	@make $@ -C $(DIR_LIST)

re: fclean all

test: $(NAME)
