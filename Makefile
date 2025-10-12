NAME = webserv
COMPILER = c++
FLAGS = -Wall -Wextra -Werror -g3 -std=c++98
SRC = src/main.cpp src/socket.cpp src/server.cpp src/cgi_poll.cpp \
      config_parsing/parse_config_file.cpp config_parsing/parsing_utils.cpp\
      config_parsing/interface_port_parser.cpp\
      config_parsing/location_parser.cpp config_parsing/server_parser.cpp\
      config_parsing/root_parser.cpp config_parsing/allowed_methods_parser.cpp\
      config_parsing/directory_listing.cpp config_parsing/default_file_parser.cpp\
      config_parsing/upload_directory_parser.cpp config_parsing/redirect_parser.cpp\
      config_parsing/max_body_size_parser.cpp config_parsing/cgi_parser.cpp\
      config_parsing/error_page_parser.cpp src/display.cpp \
      config_parsing/content_type_parser.cpp config_parsing/default_type_parser.cpp\
      src/request_parsing/body.cpp src/request_parsing/headers.cpp\
      src/request_parsing/request_line.cpp\
      src/request_parsing/HttpRequest.cpp src/response/build_response.cpp\
      src/response/delete_request.cpp src/response/get_request.cpp src/response/post_request.cpp \
      src/cgi/CgiHandler.cpp src/cgi/Environment.cpp src/cgi/cgi_helpe.cpp \
      src/cgi/pipes_files.cpp src/response/utils.cpp src/response/get_utils.cpp src/response/response_cgi.cpp

OBJDIR = obj
OBJ = $(SRC:%.cpp=$(OBJDIR)/%.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(COMPILER) $(FLAGS) $(OBJ) -o $(NAME)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(COMPILER) $(FLAGS) -c $^ -o $@

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all
