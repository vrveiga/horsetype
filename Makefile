CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lncurses -lpthread

TARGET = horsetype
SRCS = horsetype.c ascii_data.c aux_functions.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)