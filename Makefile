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
TEST_FLAGS = -Wall -Wextra -Werror -std=c++17 -I/Users/linh/.brew/include

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

# Test related variables
TEST_DIR = tests/unit
TEST_SRCS = $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJS = $(TEST_SRCS:.cpp=.o)
TEST_NAME = run_tests

# For testing we need to include the project source files excluding main.cpp
PROJECT_TEST_SRCS = $(filter-out src/main.cpp, $(SRCS))
PROJECT_TEST_OBJS = $(addprefix obj/, $(notdir $(PROJECT_TEST_SRCS:.cpp=.o)))

# Google Test flags
GTEST_FLAGS = -lgtest -lgtest_main -pthread

test: $(TEST_NAME)
	./$(TEST_NAME)

$(TEST_NAME): $(TEST_OBJS) $(PROJECT_TEST_OBJS)
	$(CC) -o $@ $^ $(FLAGS) -L/Users/linh/.brew/lib $(GTEST_FLAGS)

$(TEST_DIR)/%.o: $(TEST_DIR)/%.cpp
	$(CC) -c $(TEST_FLAGS) -o $@ $<

# Clean test artifacts
test_clean:
	rm -f $(TEST_NAME) $(TEST_OBJS)

.PHONY: clean fclean re test test_clean

clean:
	rm -f obj/*.o

fclean: clean
	rm -f $(NAME)

re: fclean all