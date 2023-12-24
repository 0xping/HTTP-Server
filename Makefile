CPP=c++
CPPFLAGS=-Wall -Wextra -Werror -std=c++98 -fsanitize=address
SRC=main.cpp src/config_parse/Config.cpp src/utils/utils.cpp
OBJ=$(SRC:.cpp=.o)
NAME=webserv

all:$(NAME)

%.o:%.cpp
	$(CPP) $(CPPFLAGS) -c $< -o $@

$(NAME):$(OBJ)
	$(CPP) $(CPPFLAGS) $(OBJ) -o $(NAME)

clean:
	rm -f $(OBJ)

fclean:clean
	rm -f $(NAME)

re:fclean all
