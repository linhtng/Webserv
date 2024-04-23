NAME = server

SRCS = main.cpp \
		Server.cpp \
		ServerManager.cpp \
		Client.cpp \
		../Request/HttpMessage.cpp \
		../Request/Request.cpp \
		../Response/Response.cpp \
		../config_parser/ConfigParser.cpp \
		../config_parser/ConfigData.cpp \
		../config_parser/Location.cpp \
		

CC = c++

FLAGS = -Wall -Wextra -Werror -std=c++17

OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) -o $@ $(OBJS) $(FLAGS)

client:
	gcc client1.c -o client1
	gcc client2.c -o client2

%.o: %.cpp
	$(CC) -c $(FLAGS) -o $@ $<

.PHONY: clean fclean re

clean:
	rm -f *.o

fclean: clean
	rm -f $(NAME)
	rm -f client1
	rm -f client2

re: fclean all client
