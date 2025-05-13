# **************************************************************************** #
#                                   Color                                      #
# **************************************************************************** #

GREEN           =   \033[0;32m
RED             =   \033[0;31m
YELLOW          =   \033[0;33m
CYAN            =   \033[1;36m
MAGENTA         =   \033[0;35m
NC              =   \033[0m

# **************************************************************************** #
#                                   Flag                                       #
# **************************************************************************** #

CXX             =   c++
CXXFLAGS        =   -Wall -Wextra -Werror -std=c++98 -I$(INC_DIR)

# **************************************************************************** #
#                                   Project                                    #
# **************************************************************************** #

NAME            =   ircserv
OBJ_DIR         =   objs
SRC_DIR         =   srcs
INC_DIR         =   includes

# **************************************************************************** #
#                                   Includes                                   #
# **************************************************************************** #

INCLUDES        =   $(INC_DIR)/Server.hpp $(INC_DIR)/Client.hpp $(INC_DIR)/Channel.hpp

# **************************************************************************** #
#                                   SRC                                        #
# **************************************************************************** #

SRC             =   $(SRC_DIR)/main.cpp $(SRC_DIR)/Server.cpp $(SRC_DIR)/Client.cpp $(SRC_DIR)/Channel.cpp $(SRC_DIR)/Commands.cpp $(SRC_DIR)/modes.cpp
OBJS            =   $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC))

# **************************************************************************** #
#                                   Rules                                      #
# **************************************************************************** #

all: $(NAME)

$(NAME): $(OBJS)
	@echo "$(CYAN)Linking $(NAME)...$(NC)"
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@echo "$(MAGENTA)$(NAME) $(GREEN)compiled successfully!$(NC)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@mkdir -p $(@D)
	
	@echo "$(GREEN)Compiling $<...$(NC)"
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

clean:
	@echo "$(RED)Cleaning up object files...$(NC)"
	@rm -rf $(OBJ_DIR)

fclean: clean
	@echo "$(RED)Removing $(NAME) binary...$(NC)"
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
