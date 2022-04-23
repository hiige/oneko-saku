PREFIX = /usr/local
BINDIR = "$(PREFIX)/bin"
CFLAGS = "-Wno-parentheses -std=c11 -pedantic -D_DEFAULT_SOURCE"
X11_LIBRARY_FLAGS = "$(shell pkg-config X11 xext || echo '-lX11 -lXext')"
LIBRARY_FLAGS = "-lc -lm $(X11_LIBRARY_FLAGS)"
LD_FLAGS = ""
CC = cc

all : oneko

oneko :
	$(CC) $(CFLAGS) oneko.c -o oneko $(LD_FLAGS) $(LIBRARY_FLAGS)

install : all
	install -o 755 oneko $(BINDIR)

clean :
	rm oneko
