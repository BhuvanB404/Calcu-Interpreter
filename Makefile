CC     = gcc
CFLAGS = -std=c89 -pedantic-errors -Wall -Wextra -Werror -g
TARGET = calc
SRCS   = main.c calc.c token.c parse.c interpriter.c
OBJS   = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c calc.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS)
