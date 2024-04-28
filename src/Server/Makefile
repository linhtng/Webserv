NAME = server

SRCS = main.cpp \
		Server.cpp \
		ServerManager.cpp \
		Client.cpp \
		../Request/Request.cpp \
		../HttpMessage/HttpMessage.cpp \
		../Response/Response.cpp \
		../config_parser/ConfigParser.cpp \
		../config_parser/ConfigData.cpp \
		../config_parser/Location.cpp \
		../StringUtils/StringUtils.cpp \
		../BinaryData/BinaryData.cpp \

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
	rm -f ../Request/*.o
	rm -f ../Response/*.o
	rm -f ../HttpMessage/*.o
	rm -f ../config_parser/*.o


fclean: clean
	rm -f $(NAME)
	rm -f client1
	rm -f client2

re: fclean all client
