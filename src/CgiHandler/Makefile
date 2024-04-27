CFLAGS = c++ -Wall -Wextra -Werror -std=c++17 -Iincludes
# DEBUG = -fsanitize=address

NAME = cgiTest

SRCS = main.cpp \
	CgiHandler.cpp \
	../config_parser/ConfigData.cpp \
	../config_parser/ConfigParser.cpp \
	../config_parser/Location.cpp \
	../Request/Request.cpp \

OBJ_PATH = objects/
OBJ		= $(SRCS:.cpp=.o)
OBJS	= $(addprefix $(OBJ_PATH), $(OBJ))

# SRCFD = sources/
# SRCS	= $(addprefix $(SRCFD), $(SRC))

all: $(NAME)

$(OBJ_PATH):
	mkdir -p $(OBJ_PATH)

$(OBJ_PATH)%.o: $(SRCFD)%.cpp
	$(CFLAGS) $(DEBUG) -c $< -o $@

$(NAME): $(OBJ_PATH) $(OBJS)
	$(CFLAGS) $(OBJS) $(DEBUG) -o $@
	
clean:
	rm -rf $(OBJ_PATH)

fclean: clean
	rm -f $(NAME)

re: fclean all
