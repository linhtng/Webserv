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
		../DefaultErrorPage/DefaultErrorPage.cpp \
		../DirectoryListingPage/DirectoryListingPage.cpp \
		../FileSystemUtils/FileSystemUtils.cpp \
		../BinaryData/BinaryData.cpp \

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

fclean: clean
	rm -f $(NAME)

re: fclean all
