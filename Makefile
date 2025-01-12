NAME=lemipc

CC=gcc
CFLAGS=-Wall -Wextra -Werror -g -O3
IFLAGS=-Imlx_linux -I/usr/include
LDFLAGS=-Lmlx_linux -lmlx_Linux -L/usr/lib -Imlx_linux -lXext -lX11 -lm -lz

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

install:
	@cd $(MLX_DIR) && make
	-@sudo cp $(MLX_DIR)/libmlx*.a /usr/X11/lib/. 2>/dev/null\
		&& echo "libmlx.a copied to /usr/X11/lib/"
	-@sudo cp $(MLX_DIR)/libmlx*.a /usr/local/lib/. 2>/dev/null\
		&& echo "libmlx.a copied to /usr/local/lib/"
	-@sudo cp $(MLX_DIR)/mlx.h /usr/X11/include/. 2>/dev/null\
		&& echo "mlx.h copied to /usr/X11/include/"
	-@sudo cp $(MLX_DIR)/mlx.h /usr/local/include/. 2>/dev/null\
		&& echo "mlx.h copied to /usr/local/include/"
	-@sudo cp $(MLX_DIR)/man/man3/mlx*.1 /usr/X11/man/man3/. 2>/dev/null\
		&& echo "man page copied to /usr/X11/man/man3/"
	-@sudo cp $(MLX_DIR)/man/man3/mlx*.1 /usr/local/man/man3/. 2>/dev/null\
		&& echo "man page copied to /usr/local/man/man3/"
	echo "requirements installed"

uninstall:
	-@sudo rm /usr/X11/lib/libmlx*.a 2>/dev/null\
		&& echo "libmlx.a removed from /usr/X11/lib/"
	-@sudo rm /usr/local/lib/libmlx*.a 2>/dev/null\
		&& echo "libmlx.a removed from /usr/local/lib/"
	-@sudo rm /usr/X11/include/mlx.h 2>/dev/null\
		&& echo "mlx.h removed from /usr/X11/include/"
	-@sudo rm /usr/local/include/mlx.h 2>/dev/null\
		&& echo "mlx.h removed from /usr/local/include/"
	-@sudo rm /usr/X11/man/man3/mlx*.1 2>/dev/null\
		&& echo "man page removed from /usr/X11/man/man3/"
	-@sudo rm /usr/local/man/man3/mlx*.1 2>/dev/null\
		&& echo "man page removed from /usr/local/man/man3/"
	echo "requirements uninstalled"

clean:
	@rm -rf $(OBJ_DIR)
	@echo $(OBJ_DIR) removed
	@cd $(MLX_DIR) && make clean

fclean: clean
	@rm -f $(NAME)
	@echo $(NAME) removed

re: fclean all

.PHONY: all install uninstall clean fclean re
