NAME = ircserv
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++17 -g
INCLUDES = -Iincludes

SRC_DIR = srcs
OBJ_DIR = objs

SRCS = main.cpp server.cpp client.cpp channel.cpp commands/cap.cpp commands/invite.cpp \
       commands/join.cpp commands/nick.cpp commands/pass.cpp commands/topic.cpp commands/ping.cpp  \
	   commands/privmsg.cpp commands/user.cpp commands/quit.cpp commands/who.cpp commands/kick.cpp \
		commands/mode.cpp \

OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all
