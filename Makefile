CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Isrc -pthread
LDFLAGS = -lncurses -pthread
TARGET = jogo
SRCDIR = src
SOURCES = $(SRCDIR)/main.c $(SRCDIR)/game.c $(SRCDIR)/ui.c $(SRCDIR)/audio.c $(SRCDIR)/fases.c
OBJECTS = $(SOURCES:.c=.o)

# Verificar se SDL2_mixer está disponível
# Primeiro tenta pkg-config, depois verifica diretamente os headers
SDL2_MIXER_PKG := $(shell pkg-config --exists sdl2_mixer 2>/dev/null && echo "yes" || echo "no")
SDL2_MIXER_HEADER := $(shell test -f /usr/include/SDL2/SDL_mixer.h && echo "yes" || echo "no")
SDL2_MIXER_HEADER_ALT := $(shell test -f /usr/local/include/SDL2/SDL_mixer.h && echo "yes" || echo "no")

ifeq ($(SDL2_MIXER_PKG),yes)
    # Usa pkg-config se disponível
    CFLAGS += $(shell pkg-config --cflags sdl2_mixer)
    LDFLAGS += $(shell pkg-config --libs sdl2_mixer)
    # Garante que SDL2 também está linkado
    SDL2_LIB := $(shell pkg-config --libs SDL2 2>/dev/null || echo "-lSDL2")
    LDFLAGS += $(SDL2_LIB)
    CFLAGS += -DHAVE_SDL2_MIXER
else ifeq ($(SDL2_MIXER_HEADER),yes)
    # Headers encontrados em /usr/include/SDL2/
    CFLAGS += -I/usr/include/SDL2 -DHAVE_SDL2_MIXER
    LDFLAGS += -lSDL2_mixer -lSDL2
else ifeq ($(SDL2_MIXER_HEADER_ALT),yes)
    # Headers encontrados em /usr/local/include/SDL2/
    CFLAGS += -I/usr/local/include/SDL2 -DHAVE_SDL2_MIXER
    LDFLAGS += -lSDL2_mixer -lSDL2
else
    # Tenta verificar se o header existe em qualquer lugar
    SDL2_MIXER_HEADER_FOUND := $(shell find /usr/include /usr/local/include -name "SDL_mixer.h" 2>/dev/null | head -1)
    ifneq ($(SDL2_MIXER_HEADER_FOUND),)
        CFLAGS += -DHAVE_SDL2_MIXER
        LDFLAGS += -lSDL2_mixer -lSDL2
    endif
endif

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

