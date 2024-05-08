NAME = webserv

SRC_FILENAMES = main.cpp \
		CgiHandler/CgiHandler.cpp \
		Config/ConfigParser.cpp \
		Config/ConfigData.cpp \
		Config/Location.cpp \
		Server/Server.cpp \
		Server/ServerManager.cpp \
		Server/Client.cpp \
		HttpMessage/HttpMessage.cpp \
		Request/Request.cpp \
		Response/Response.cpp \
		Utils/StringUtils.cpp \
		Utils/FileSystemUtils.cpp \
		Utils/BinaryData.cpp \
		Utils/Logger.cpp

CC = c++

FLAGS = -Wall -Wextra -Werror -std=c++17

SRCS = $(addprefix src/, $(SRC_FILENAMES))

OBJS = $(addprefix obj/, $(notdir $(SRCS:.cpp=.o)))

all: obj $(NAME)

obj:
	mkdir -p obj

$(NAME): $(OBJS)
	$(CC) -o $@ $(OBJS) $(FLAGS)

vpath %.cpp $(sort $(dir $(SRCS)))

obj/%.o: %.cpp
	$(CC) -c $(FLAGS) -o $@ $<

.PHONY: clean fclean re

clean:
	rm -f obj/*.o

fclean: clean
	rm -f $(NAME)

re: fclean all