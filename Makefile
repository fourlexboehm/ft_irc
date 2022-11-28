
NAME  = ircserv
CC    = c++
FLAGS = -Wall -Wextra -Werror -std=c++98 -g
RM	  = rm -f
 
SRCS  =      main.cpp src/Server.cpp src/commands.cpp
OBJS  = ${SRCS:.cpp=.o}

.cpp.o:
	${CC} ${FLAGS} -I./include -c $< -o ${<:.cpp=.o}


${NAME}:	${OBJS}
			${CC} ${FLAGS} -I./include -o ${NAME} ${OBJS}

all:		${NAME}

test: re
	./ircserv 6667 "1"

bonus:		all

clean:
			@ ${RM} $(OBJS)

fclean:		clean
			@ ${RM} ${NAME}

re:			fclean all

.PHONY:		all clean fclean re


