################################### VARIABLES ##################################
## Variables can only be strings:
# e.g.:
#      a = one two #a is assigned to the string "one two"
# Reference to variables using either ${} or $()
#
## There are two flavors of variables:
# - recursive (use =) -> only looks for the variables when the command is used, not when it's defined.
# - simply expanded (use :=) -> like normal imperative programming. Only those defined so far get expanded.
## e.g.:
# one = one ${later_variable} -> will look for later_variable when one is used.
# two := two ${later_variable} -> later_variable is not yet defined.
# later_variable = later -> The variable defined after one and two variables.
# all:
#	echo $(one) -> This will print "one later"
#	echo $(two) -> This will print "two" (will not print the later_variable)
#
## := allows you to append to a variable:
# e.g.: var3 := ${var1} file ${var2}
# or
# foo := one
# foo += two
#
# ?= only sets variables if they have not yet been set

NAME = webserv

CPP = c++

FLAGS = -Iincludes -Wall -Werror -Wextra -pedantic -std=c++98 -Wshadow -g3 -fsanitize=address
# other debug options: -fsanitize=address -g3
# -pedantic -> Issue all the warnings demanded by strict ISO, it follows the
#  version of the ISO standard specified by any -std option.
# -Wshadow -> if variables, types, typedefs and/or parameters... have the same
#  name (i.e. shadows one another).
# -Wno-shadow -> is the default for the compiler, determinating that no warning
#  is issued when variables are being shadowed.

# Any modify to those files will alert Makefile's rules and recompile
INCLUDES_DEPENDENCY = ./includes/*.hpp standard_complete.conf

SRC =   srcs/ConfigFileParser.cpp\
		srcs/Parser.cpp\
		srcs/RequestParserURLpath.cpp\
		srcs/RequestParser.cpp\
		srcs/ServerData.cpp\
		srcs/ServerLocation.cpp\
		srcs/RequestData.cpp\
		srcs/ResponseData.cpp\
		srcs/CgiData.cpp\
		srcs/WebServer.cpp\
		tests/Kqueue_main.cpp\
		srcs/SigHandler.cpp

OBJ = $(SRC:%.cpp=%.o)

##### COLORS and ACTIONS #####
# web.archive.org/web/20190624214929/http://www.termsys.demon.co.uk/vtansi.htm
WHITE = \033[1;37m
GREY = \033[1;30m
RED = \033[1;31m
GREEN = \033[1;32m
YELLOW = \033[1;33m
LIGHT_BLUE = \033[1;36m
RESET = \033[0m
CLEAR_SCREEN = \033c
ERASE_LINE = \033[2K
JUMP_ONE_LINE = \033[1B

CONFIG_FILE = standard_complete.conf
#################################### RULES ####################################
## Makefile Syntax of a rule:
# targets: prerequisites
#	command
#	command
# - The targets are file names, separated by spaces. A target will be run only
#   if its file does not exist or if its prerequisites are newer than itself.
# - The prerequisites (or dependencies) are also file names separated by spaces.
#   They need to exist before the commands for the target are run.
# - The commands are a series of steps typically used to make the target(s).
#   These need to start with a tab character, not spaces.
#
## Automatic variables:
# $@ = The target of the rule -> $(NAME)
# $< = The first prerequisite
# $^ = All prerequisites
# $? = All prerequisites NEWER than the target
# https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html

all: $(NAME)

$(NAME): $(OBJ) $(INCLUDES_DEPENDENCY)
	$(CPP) $(FLAGS) -o $@ $(OBJ)
	@echo "$(GREEN)server loaded$(RESET)"
	@echo "$(GREEN)run ./webserv [file.conf]$(RESET)"

# - Pattern rules contain a '%' in the target: creates a stem by matching
#   any nonempty string (the other character(s) have to match themselves).
# - In a prerequisite, '%' stands for the same stem matched by the target.
%.o: %.cpp $(INCLUDES_DEPENDENCY)
	$(CPP) $(FLAGS) -c $< -o $@
# -c = Compile or assemble the source files ($<)
#      $< for the $(SRCPATH)/%.cpp part (the first prerequisite)
# generating an .o object file (with the use of -o) for each source file given.
#      $@ for the $(OBJPATH)/%.o part (the target).

################################# CLEAN RULES #################################

clean:
	rm -rf $(OBJ)
	@rm -rf a.out *.dSYM
	@rm -rf confiFileTester
	@echo "$(YELLOW)clean done$(RESET)"

fclean: clean
	rm -f $(NAME)
	@echo "$(YELLOW)fclean done$(RESET)"

re: fclean all

# ex: make config CONFIG_FILE=README.md
config: $(CONFIG_FILE)
	$(CPP) $(FLAGS) tests/mainConfigFile.cpp srcs/Parser.cpp srcs/ConfigFileParser.cpp srcs/ServerData.cpp srcs/ServerLocation.cpp -o confiFileTester
	@echo "$(GREEN)Testing $(CONFIG_FILE)$(RESET)\n"
	@./confiFileTester $(CONFIG_FILE)

.PHONY: all clean fclean re config
# PHONY = Prevents make from confusing the phony target with a file name.
# i.e.: Use PHONY if a target is not intended to be a filename:
# e.g.: If you happen to have a file named clean, this target won't run, which
# is not what we want.