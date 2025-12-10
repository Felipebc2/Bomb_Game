CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Isrc
LDFLAGS = -lncurses
TARGET = jogo
SRCDIR = src
SOURCES = $(SRCDIR)/main.c $(SRCDIR)/game.c $(SRCDIR)/ui.c
OBJECTS = $(SOURCES:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

