CC = gcc
CFLAGS = -O2 -Wall -Wextra
TARGET = stringperformance
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) -lm

clean:
	rm -f $(TARGET)

.PHONY: all clean