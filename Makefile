NAME=lemipc

CC=gcc
CFLAGS=-Wall -Wextra -Werror -g -O3
IFLAGS=-Imlx_linux -I./mlx
LDFLAGS=-Lmlx_linux -lmlx_Linux -L./mlx -Imlx_linux -lXext -lX11 -lm -lz

INCLUDE_DIR=include
INCLUDE_EXT=h
INCLUDE=$(shell find $(INCLUDE_DIR) -type f -name "*.$(INCLUDE_EXT)")

SRC_EXT=c
SRC_DIR=src
SRC=$(shell find $(SRC_DIR) -type f -name "*.$(SRC_EXT)")

OBJ_DIR=obj
OBJ=$(patsubst $(SRC_DIR)/%.$(SRC_EXT),$(OBJ_DIR)/%.o,$(SRC))

MLX_DIR=mlx

all: $(NAME)

$(NAME): $(OBJ_DIR) $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS)
	@echo $(NAME) created

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.$(SRC_EXT) $(INCLUDE) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ $(IFLAGS)

clean:
	@rm -rf $(OBJ_DIR)
	@echo $(OBJ_DIR) removed

fclean: clean
	@rm -f $(NAME)
	@echo $(NAME) removed

mlx:
	@cd $(MLX_DIR) && make

mlx-clean:
	@cd $(MLX_DIR) && make clean

re: fclean all

.PHONY: all clean fclean re mlx mlx-clean
