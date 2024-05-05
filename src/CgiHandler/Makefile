NAME = cgi_test

SRCS = main.cpp \
		CgiHandler.cpp \
		../Server/Server.cpp \
		../Server/Client.cpp \
		../HttpMessage/HttpMessage.cpp \
		../Request/Request.cpp \
		../Response/Response.cpp \
		../config_parser/ConfigParser.cpp \
		../config_parser/ConfigData.cpp \
		../config_parser/Location.cpp \
		../StringUtils/StringUtils.cpp \
		../FileSystemUtils/FileSystemUtils.cpp \
		../BinaryData/BinaryData.cpp \
		../Logger/Logger.cpp \

CC = c++

FLAGS = -Wall -Wextra -Werror -std=c++17

OBJS = $(addprefix obj/, $(notdir $(SRCS:.cpp=.o)))

vpath %.cpp $(sort $(dir $(SRCS)))

all: obj $(NAME)

obj:
	mkdir -p obj

$(NAME): $(OBJS)
	$(CC) -o $@ $(OBJS) $(FLAGS)

obj/%.o: %.cpp
	$(CC) -c $(FLAGS) -o $@ $<

.PHONY: clean fclean re

clean:
	rm -f obj/*.o

fclean: clean
	rm -f $(NAME)

re: fclean all