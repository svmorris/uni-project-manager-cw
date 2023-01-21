CC = gcc
CFLAGS = -Wall -pedantic -std=gnu17

# Debug version of cflags: adds debugging symbols and address sanitizer
DBG_FLAGS = -g -fsanitize=address

# name of output binary
OUTPUT = pm

# 'local' (or my own) libraries
LIBS = -lproject -lutil -lfilesystem -lserver
LIBS_ARG = -Isrc -Llib



create_check_dirs:
	-@mkdir bin  2> /dev/null
	-@mkdir lib  2> /dev/null
	-@mkdir doc  2> /dev/null
	-@mkdir conf  2> /dev/null


test:
	@echo "Runnin all tests..."
	cd tests && ./all.sh


dev: | create_check_dirs
	@# @# Build the program with the debug CFLAGS
	@echo -e "\nBuilding project for debug..."
	@echo -e "\nCC project.c"
	@${CC} ${CFLAGS} ${DBG_FLAGS} -c src/project.c -o lib/project.o
	@ar rv lib/libproject.a lib/project.o
	@echo -e "\nCC util.c"
	@${CC} ${CFLAGS} ${DBG_FLAGS} -c src/util.c -o lib/util.o
	@ar rv lib/libutil.a lib/util.o
	@echo -e "\nCC filesystem.c"
	@${CC} ${CFLAGS} ${DBG_FLAGS} -c src/filesystem.c -o lib/filesystem.o
	@ar rv lib/libfilesystem.a lib/filesystem.o
	@echo -e "\nCC server.c"
	@${CC} ${CFLAGS} ${DBG_FLAGS} -c src/server.c -o lib/server.o
	@ar rv lib/libserver.a lib/server.o
	@echo -e "\nCC main.c"
	@${CC} ${CFLAGS} ${DBG_FLAGS} src/main.c -o bin/${OUTPUT} ${LIBS_ARG} ${LIBS}
	@echo -en "\nBuilt successfully!"

build: | create_check_dirs
	@# Build project for production
	@echo -e "\nBuilding project for prod..."
	@echo -e "\nCC project.c"
	@${CC} ${CFLAGS} -c src/project.c -o lib/project.o
	@ar rv lib/libproject.a lib/project.o
	@echo -e "\nCC util.c"
	@${CC} ${CFLAGS}  -c src/util.c -o lib/util.o
	@ar rv lib/libutil.a lib/util.o
	@echo -e "\nCC filesystem.c"
	@${CC} ${CFLAGS}  -c src/filesystem.c -o lib/filesystem.o
	@ar rv lib/libfilesystem.a lib/filesystem.o
	@echo -e "\nCC server.c"
	@${CC} ${CFLAGS}  -c src/server.c -o lib/server.o
	@ar rv lib/libserver.a lib/server.o
	@echo -e "\nCC main.c"
	@${CC} ${CFLAGS}  src/main.c -o bin/${OUTPUT} ${LIBS_ARG} ${LIBS}
	@echo -en "\nBuilt successfully!"


docs: | create_check_dirs
	doxygen

clean:
	@echo -e "\nRemoving build files..."
	-rm -rf doc/html
	-rm -rf doc/latex
	-rm -rf bin
	-rm -rf lib
	-rm -rf conf


all: clean build
