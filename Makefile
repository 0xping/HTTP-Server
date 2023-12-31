CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -fsanitize=address
NAME = webserv

SRC = $(wildcard utils/*.cpp src/*/**.cpp src/*.cpp) main.cpp
OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME):$(OBJ)
	$(CPP) $(CPPFLAGS) $(OBJ) -o $(NAME)

clean:
	rm -f $(OBJ)

fclean:clean
	rm -f $(NAME)

re:fclean all
