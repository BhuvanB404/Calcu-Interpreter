CC = gcc
CFLAGS = -std=c89 -pedantic-errors -Wall -Wextra -Werror -g
TARGET = calc

all: $(TARGET)

$(TARGET): main.c calc.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c calc.c

clean:
	rm -f $(TARGET)


