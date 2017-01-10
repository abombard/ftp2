CC=clang
FLAGS=
FLAGS42=-Wall -Wextra -Werror

NAME=liblist.a

SRC_DIR=srcs
INCLUDES=-I ./

BUILD=__build

SRC=\
	t_list_add.c		\
	t_list_del.c		\
	t_list_splice.c		\
	list_push_front.c	\
	list_push_back.c	\
	list_del.c			\
	list_is_empty.c		\
	list_insert.c		\
	list_size.c			\
	list_move.c			\
	list_nth.c			\
	list_nth_safe.c		\
	list_slice.c		\
	list_splice.c		\

OBJ=$(addprefix $(BUILD)/,$(SRC:.c=.o))

all:$(BUILD) $(NAME)

$(BUILD):
	mkdir -p $@

$(BUILD)/%.o:$(SRC_DIR)/%.c
	@$(CC) $(FLAGS42) -c $< -o $@ $(INCLUDES)

$(NAME):$(OBJ)
	@ar rc $@ $(OBJ)
	@ranlib $@
	@echo "$@ was created"

clean:
	@rm -rf $(BUILD)

fclean: clean
	@rm -f $(NAME)

re: fclean all
