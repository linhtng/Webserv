NAME = request_test

SRCS = main.cpp \
		../Server/Server.cpp \
		../Server/Client.cpp \
		../HttpMessage/HttpMessage.cpp \
		Request.cpp \
		../Response/Response.cpp \
		../config_parser/ConfigParser.cpp \
		../Config/ConfigData.cpp \
		../config_parser/Location.cpp \
		../StringUtils/StringUtils.cpp \
		../BinaryData/BinaryData.cpp \
		../FileSystemUtils/FileSystemUtils.cpp \
		../Logger/Logger.cpp

CC = c++

FLAGS = -Wall -Wextra -Werror -std=c++17

OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) -o $@ $(OBJS) $(FLAGS)

%.o: %.cpp
	$(CC) -c $(FLAGS) -o $@ $<

.PHONY: clean fclean re

clean:
	rm -f *.o
	rm -f ../Server/*.o
	rm -f ../HttpMessage/*.o
	rm -f ../Response/*.o
	rm -f ../config_parser/*.o
	rm -f ../StringUtils/*.o
	rm -f ../BinaryData/*.o
	rm -f ../FileSystemUtils/*.o
	rm -f ../Logger/*.o

fclean: clean
	rm -f $(NAME)

re: fclean all
