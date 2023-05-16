CC = gcc
CFLAGS = -Wall -Wextra -Ilib -Ilibuv/include
LDFLAGS = -L./ -Llibuv -Llib
LIBS = -luv -lws2_32

SERVER_SRCS = server.c
RESPONSE_SRCS = lib/response.c

SERVER_OBJS = $(SERVER_SRCS:.c=.o)
RESPONSE_OBJS = response.o

all: server

server: $(SERVER_OBJS) $(RESPONSE_OBJS)
	$(CC) $(CFLAGS) -o server $(SERVER_OBJS) $(RESPONSE_OBJS) $(LDFLAGS) $(LIBS)

$(SERVER_OBJS): $(SERVER_SRCS)
	$(CC) $(CFLAGS) -c $(SERVER_SRCS)

$(RESPONSE_OBJS): $(RESPONSE_SRCS)
	$(CC) $(CFLAGS) -c $(RESPONSE_SRCS)

clean:
	del server $(SERVER_OBJS) $(RESPONSE_OBJS)
