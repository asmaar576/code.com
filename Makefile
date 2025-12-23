# Makefile for UNO Raylib Project (Windows/MSYS2)

CC = gcc
CFLAGS = -Wall -O2
LDFLAGS = -lraylib -lopengl32 -lgdi32 -lwinmm -lws2_32

# Object files
OBJ = main.o core.o log.o gui.o

# Target executable
TARGET = uno_gui.exe

# Build rules
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

main.o: main.c core.h log.h gui.h
	$(CC) $(CFLAGS) -c main.c

core.o: core.c core.h
	$(CC) $(CFLAGS) -c core.c

log.o: log.c log.h core.h
	$(CC) $(CFLAGS) -c log.c

gui.o: gui.c gui.h core.h
	$(CC) $(CFLAGS) -c gui.c

# Clean compiled files
clean:
	del /Q $(OBJ) $(TARGET)
