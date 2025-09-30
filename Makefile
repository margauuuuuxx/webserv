NAME		= webserv 

SRCS 		= 	srcs/CGI.cpp srcs/CGIutils.cpp srcs/main.cpp srcs/Parser.cpp \
				srcs/Poller.cpp srcs/Request.cpp srcs/Response.cpp \
				srcs/responseUtils.cpp srcs/Server.cpp srcs/Socket.cpp \
				srcs/SocketArray.cpp srcs/utils.cpp
OBJS_DIR	= objs
INC_DIR		= includes
OBJS		= $(patsubst srcs/%.cpp, $(OBJS_DIR)/%.o, $(SRCS))
FLAGS 		= -std=c++98 -g -fstack-protector-all -Wall -Wextra -Werror -DDEBUG_MODE #look here !!!
INCLUDES	= -I$(INC_DIR)

RED   		= \033[1;38;5;196m
PINK  		= \033[1;38;5;213m
NC			= \033[0m
FACE		= 😏

all: $(NAME)

$(NAME): $(OBJS)
	@c++ $(FLAGS) $(OBJS) -o $(NAME)
	@printf "\r${PINK}Compiiiiiling ${FACE} : [${RED}%-50s${RED}${PINK}] %d/%d${NC}" $$(printf "#%.0s" $$(seq 1 $$(expr $$(find $(OBJS_DIR) -name '*.o' | wc -l) \* 50 / $(words $(SRCS))))) $$(find $(OBJS_DIR) -name '*.o' | wc -l) $(words $(SRCS))

$(OBJS_DIR)/%.o: srcs/%.cpp
	@mkdir -p $(OBJS_DIR)
	@c++ $(FLAGS) $(INCLUDES) -c $< -o $@

clean:
	@rm -rf $(OBJS_DIR)
	@rm -rf www/uploads/*

fclean: clean
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re clean_uploads