NAME	=	webserv
FLAGS	=	-Wall -Wextra -Werror -g -std=c++98
FILES	=	$(wildcard srcs/*.cpp)

all : ${NAME}

${NAME} : ${FILES}
	@c++ ${FLAGS} ${FILES} -o ${NAME}

clean :
	@rm -rf ${NAME}

fclean : clean

re : fclean all

.PHONY:	all clean fclean re