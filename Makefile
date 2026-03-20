<<<<<<< Updated upstream
CC     = gcc
CFLAGS = -std=c89 -pedantic-errors -Wall -Wextra -Werror -g
TARGET = calc
SRCS   = main.c calc.c token.c parse.c interpreter.c
OBJS   = $(SRCS:.c=.o)
=======
CC       = gcc
CFLAGS   = -std=c89 -pedantic-errors -Wall -Wextra -Werror -g
LDFLAGS  = -lm
TARGET   = calc
BUILDDIR = build
SRCS     = main.c calc.c error.c token.c parse.c interpreter.c
OBJS     = $(SRCS:%.c=$(BUILDDIR)/%.o)
>>>>>>> Stashed changes

all: $(BUILDDIR) $(TARGET)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

<<<<<<< Updated upstream
%.o: %.c calc.h
=======
$(BUILDDIR)/%.o: %.c calc.h error.h token.h parse.h interpreter.h
>>>>>>> Stashed changes
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(TARGET) $(BUILDDIR)
