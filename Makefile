CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98  # -g -fsanitize=address -o3
NAME = webserv

SRC = $(wildcard utils/*.cpp src/*/**.cpp src/*.cpp) main.cpp
OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(NAME)

clean:
	@rm -rf $(OBJ)

fclean: clean
	@rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re