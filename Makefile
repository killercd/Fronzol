CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O3 -flto
LDLIBS = -lssl -lcrypto
TARGET = fronzol
OBJS = fronzol.o browser.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS)

fronzol.o: fronzol.c browser.h
	$(CC) $(CFLAGS) -c fronzol.c

browser.o: browser.c browser.h
	$(CC) $(CFLAGS) -c browser.c

clean:
	rm -f $(TARGET) $(OBJS)
