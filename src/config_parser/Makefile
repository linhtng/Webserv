CFLAGS = c++ -Wall -Wextra -Werror -std=c++17 -Iincludes
# DEBUG = -fsanitize=address

NAME = webserv

SRCS = srcs/config_parser/main.cpp \
	srcs/config_parser/ConfigParser.cpp \

# OBJ_PATH = objects/
OBJ		= $(SRCS:.cpp=.o)
# OBJS	= $(addprefix $(OBJ_PATH), $(OBJ))

# SRCFD = sources/
# SRCS	= $(addprefix $(SRCFD), $(SRC))

all: $(NAME)

$(OBJ_PATH):
	mkdir -p $(OBJ_PATH)

$(OBJ_PATH)%.o: $(SRCFD)%.cpp
	$(CFLAGS) $(DEBUG) -c $< -o $@

$(NAME): $(OBJ_PATH) $(OBJ)
	$(CFLAGS) $(OBJ) $(DEBUG) -o $@
	
clean:
	rm $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all
