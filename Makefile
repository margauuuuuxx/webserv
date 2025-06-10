NAME	=	webserv
FLAGS	=	-Wall -Wextra -Werror -std=c++98
FILES	=	srcs/main.cpp

all : ${NAME}

${NAME} : ${FILES}
	@c++ ${FLAGS} ${FILES} -o ${NAME}

clean :
	@rm -rf ${NAME}

fclean : clean

re : fclean all

.PHONY:	all clean fclean re