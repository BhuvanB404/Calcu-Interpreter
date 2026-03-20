CC       = gcc
CFLAGS   = -std=c89 -pedantic-errors -Wall -Wextra -Werror -g
LDFLAGS  = -lm
TARGET   = calc
BUILDDIR = build
SRCS     = main.c calc.c error.c token.c parse.c interpreter.c
OBJS     = $(SRCS:%.c=$(BUILDDIR)/%.o)

all: $(BUILDDIR) $(TARGET)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

$(BUILDDIR)/%.o: %.c calc.h error.h token.h parse.h interpreter.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(TARGET) $(BUILDDIR)
