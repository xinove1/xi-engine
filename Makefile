NAME= game

RAYLIB= ./external/raylib-5.0/src

CC= gcc
CC_WINDOWS= x86_64-w64-mingw32-gcc

XILIB = libxilib.so

CFLAGS= -I$(RAYLIB) -I$(RAYLIB)/external -I./src/modules/ -std=c99
DEBUG_FLAGS= -g3 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function \
             -fsanitize=address -fsanitize=undefined -fsanitize-trap \
             #-Wconversion  -Wno-sign-conversion -Werror -Wdouble-promotion \

RFLAGS= -lGL -lm -lpthread -ldl -lrt -lX11
RFLAGS_WINDOWS= -lopengl32 -lgdi32 -lwinmm

WEB_DATA_DIR= --preload-file assets
WEB_EXPORTED_FUNCTIONS= -sEXPORTED_FUNCTIONS=_main
WEBFLAGS = $(WEB_EXPORTED_FUNCTIONS) $(WEB_DATA_DIR) --shell-file ./template.html --js-library lib.js -s ALLOW_MEMORY_GROWTH=1 -s EXPORTED_RUNTIME_METHODS=ccall,cwrap -s STACK_SIZE=1mb -Os -s USE_GLFW=3 -DPLATFORM_WEB -sGL_ENABLE_GET_PROC_ADDRESS

SRC_ENGINE = src/main.c
SRC_GAME = $(wildcard src/game_code/*.c)
SRC= $(SRC_ENGINE) $(SRC_GAME) src/implementations.c

OBJ_ENGINE=  OBJ_GAME= $(notdir $(SRC_GAME:.c=.o))
OBJ= $(notdir $(SRC:.c=.o))

DEPENDENCIES= $(SRC) $(wildcard src/game_code/*.h) $(wildcard src/modules/*.h)

RM= rm -f

# $(NAME): $(DEPENDENCIES)
# 	@make -C $(RAYLIB) RAYLIB_LIBTYPE=SHARED
# 	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -c $(SRC)
# 	$(CC) $(OBJ) $(CFLAGS) $(DEBUG_FLAGS) -Wl,-rpath=$(RAYLIB)/ -lraylib $(RFLAGS) -o $(NAME)

HOT_FLAGS = -DHOT_RELOAD -Wl,-rpath=$(RAYLIB)/ -Wl,-rpath=./ -lraylib -L.

hot: $(DEPENDANCIES) src/game_code/game.h
	@make -C $(RAYLIB) RAYLIB_LIBTYPE=SHARED
	$(MAKE) $(XILIB)
	$(MAKE) $(NAME).so 
	$(MAKE) $(NAME) 

$(NAME): $(SRC_ENGINE) $(wildcard src/modules/*.h)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(RFLAGS) $(HOT_FLAGS) -lxilib $(SRC_ENGINE) -o $(NAME)

$(NAME).so: $(SRC_GAME) $(wildcard src/game_code/*.h)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(HOT_FLAGS) -fpic -shared $(SRC_GAME) -o $(NAME).so

$(XILIB): src/implementations.c $(wildcard src/modules/*.h)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -fpic -shared src/implementations.c -o $(XILIB)

static: $(DEPENDENCIES)
	@make -C $(RAYLIB)
	$(CC) $(CFLAGS) -c $(SRC)
	$(CC) $(OBJ) $(RAYLIB)/libraylib.a $(CFLAGS) $(RFLAGS) -o $(NAME)

cosmos: $(DEPENDENCIES)
	@make -C $(RAYLIB) CC=cosmocc
	cosmocc $(CFLAGS) -c $(SRC)
	cosmocc $(OBJ) $(RAYLIB)/libraylib.a $(CFLAGS) $(RFLAGS) -o $(NAME)

windows: $(DEPENDENCIES)
	@make -C $(RAYLIB) OS=Windows_NT CC=x86_64-w64-mingw32-gcc AR=x86_64-w64-mingw32-ar
	$(CC_WINDOWS) $(CFLAGS) -c $(SRC)
	$(CC_WINDOWS) $(OBJ) $(RAYLIB)/libraylib.a $(RFLAGS_WINDOWS) -o $(NAME).exe

web: $(DEPENDENCIES)
	make -C $(RAYLIB) PLATFORM=PLATFORM_WEB -B EMSDK_PATH=/home/xinove/stuff/emsdk  PYTHON_PATH=/usr/bin/python NODE_PATH=/home/xinove/stuff/emsdk/node/16.20.0_64bit/bin
	emcc $(CFLAGS) -DPLATFORM_WEB -c $(SRC)
	emcc $(OBJ) $(RAYLIB)/libraylib.a $(CFLAGS) $(RFLAGS) $(WEBFLAGS) -DPLATFORM_WEB -o $(NAME).html
#emcc -o game.html game.c -Os -Wall ./path-to/libraylib.a -I. -Ipath-to-raylib-h -L. -Lpath-to-libraylib-a  --shell-file path-to/shell.html

web_run: $(web)
	emrun ./index.html

run: hot
	./$(NAME)

all: $(NAME)

bear: 
	bear -- make

cleana: 
	$(RM) $(OBJ)
	$(RM) $(NAME)
	$(RM) $(NAME).so
	$(RM) $(XILIB)

clean:
	$(RM) $(OBJ)

fclean: clean
	@make -C $(RAYLIB) clean
	$(RM) $(NAME)
	$(RM) $(XILIB)
	$(RM) $(NAME).so
	$(RM) $(NAME).html
	$(RM) $(NAME).js
	$(RM) $(NAME).wasm
	$(RM) $(NAME).exe
	$(RM) $(NAME).data


re: fclean all

.PHONY: clean fclean e ew
