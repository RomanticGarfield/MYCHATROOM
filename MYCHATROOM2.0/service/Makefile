PROGRAM = Service

CC = gcc
C_FLAGS = -std=c99 -Wall -g -I -DDEBUG -lpthread

C_SOURCES = service.c chat.c
#C_OBJECTS = $(patsubst %.c,%.o, $(C_SOURCE))


all:$(C_SOURCES)
	@echo compile...
	$(CC) $(C_SOURCES) $(C_FLAGS) -o $(PROGRAM)

.PHONY:clean
clean:
	rm $(PROGRAM)
