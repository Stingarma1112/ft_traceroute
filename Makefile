# ===============================
#  Makefile — ft_ping
# ===============================

NAME 	:= ft_ping

SRC_DIR := src
INC_DIR := include
OBJ_DIR := .obj

SRC 	:= $(addprefix $(SRC_DIR)/, main.c) \
		   $(addprefix $(SRC_DIR)/, display.c) \
		   $(addprefix $(SRC_DIR)/, dns.c) \
		   $(addprefix $(SRC_DIR)/, icmp.c) \
		   $(addprefix $(SRC_DIR)/, parsing.c) \
		   $(addprefix $(SRC_DIR)/, receive.c) \
		   $(addprefix $(SRC_DIR)/, send.c) \
		   $(addprefix $(SRC_DIR)/, socket.c) \
		   $(addprefix $(SRC_DIR)/, stats.c) \
		   $(addprefix $(SRC_DIR)/, utils.c)

OBJ 	:= $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CC 		:= cc
CFLAGS  := -Wall -Wextra -Werror -g3 -I$(INC_DIR)

GREEN := \033[32m
YELLOW := \033[33m
RESET := \033[0m

TOTAL_SOURCES := $(words $(SRC))
CURRENT := 0

.PHONY: all clean fclean re OBJ_DIR

all: $(NAME)
	@echo "$(GREEN)✅ Compilation completed successfully!$(RESET)"

$(NAME): $(OBJ)
	@echo -n "🔗 Linking executable... "
	@$(CC) $(OBJ) -o $(NAME) -lm 2>/dev/null
	@echo "Done!"

OBJ_DIR:
	@mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | OBJ_DIR
	@$(eval CURRENT := $(shell echo $$(($(CURRENT) + 1))))
	@echo -n "📦 Compiling [$(CURRENT)/$(TOTAL_SOURCES)] "
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Done!"

clean:
	@echo -n "$(YELLOW)🧹 Cleaning object files... $(RESET)"
	@rm -rf $(OBJ_DIR) 2>/dev/null
	@echo "$(YELLOW)Done!$(RESET)"

fclean: clean
	@echo -n "$(YELLOW)🗑️  Removing executable... $(RESET)"
	@rm -rf $(NAME) 2>/dev/null
	@echo "$(YELLOW)Done!$(RESET)"
	@echo "$(YELLOW)✨ All files cleaned successfully!$(RESET)"

re: fclean all