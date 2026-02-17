CC        = cc
CFLAGS    = -Wall -Wextra -Werror -Iinclude
SDL       = -lSDL2

SRCS_DIR  = src/
OBJS_DIR  = obj/

GREEN  = \033[38;2;60;245;39m
PINK   = \033[38;2;245;39;128m
RED    = \033[38;2;237;26;26m
CYAN   = \033[1;36m
YELLOW = \033[1;33m
RESET  = \033[0m

SRC_SERVER = $(wildcard $(SRCS_DIR)server/*.c)
SRC_GAME = $(wildcard $(SRCS_DIR)pingpong/*.c)

OBJ_SERVER = $(patsubst $(SRCS_DIR)server/%.c,$(OBJS_DIR)server/%.o,$(SRC_SERVER))
OBJ_GAME = $(patsubst $(SRCS_DIR)pingpong/%.c,$(OBJS_DIR)pingpong/%.o,$(SRC_GAME))

SERVER_NAME = server
GAME_NAME = pingpong


all: server game

server: $(OBJ_SERVER)
	@echo "$(YELLOW)🔗 Linking $(SERVER_NAME)...$(RESET)"
	@$(CC) $(CFLAGS) $(OBJ_SERVER) -o $(SERVER_NAME) $(SDL) -lm
	@echo "$(GREEN)Build complete: ./$(SERVER_NAME)$(RESET)"

game: $(OBJ_GAME)
	@echo "$(YELLOW)🔗 Linking $(GAME_NAME)...$(RESET)"
	@$(CC) $(CFLAGS) $(OBJ_GAME) -o $(GAME_NAME) $(SDL) -lm
	@echo "$(GREEN)Build complete: ./$(GAME_NAME)$(RESET)"

$(OBJS_DIR)server/%.o: $(SRCS_DIR)server/%.c
	@mkdir -p $(dir $@)
	@echo "$(CYAN)[Compiling]$(RESET) $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJS_DIR)pingpong/%.o: $(SRCS_DIR)pingpong/%.c
	@mkdir -p $(dir $@)
	@echo "$(CYAN)[Compiling]$(RESET) $<"
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -rf $(OBJS_DIR)
	@echo "$(RED)🧹 Object files removed!$(RESET)"

fclean: clean
	@rm -f $(SERVER_NAME) $(GAME_NAME)
	@echo "$(RED)🔥 Executables removed$(RESET)"

re: fclean all

.PHONY: all clean fclean re server game
