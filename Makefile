SERVER_DIR=server_srcs
CLIENT_DIR=client_srcs

SERVER=server
CLIENT=client

all:
	@make -C $(SERVER_DIR)
	@cp $(SERVER_DIR)/$(SERVER) .
	@make -C $(CLIENT_DIR)
	@cp $(CLIENT_DIR)/$(CLIENT) .

clean:
	@make $@ -C $(SERVER_DIR)
	@make $@ -C $(CLIENT_DIR)

fclean:clean
	@make $@ -C $(SERVER_DIR)
	@make $@ -C $(CLIENT_DIR)
	@rm -f $(SERVER) $(CLIENT)

re:fclean all
