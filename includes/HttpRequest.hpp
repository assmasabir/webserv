#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "webserv.hpp"
#include "parse_output.hpp"
class CgiHandler;

typedef struct RequestData
{
  std::string 							method;
  std::string 							path;
  std::string 							query;
  std::string 							version;
  std::map<std::string, std::string>	headers;
  std::string 							body;
} requestdata;

class HttpRequest
{
	private:
		requestdata 				requestData;
		bool						body;
		int							content_length;
		int							port;
		std::string					interface;
		locationconfig				location;
		serverconfig				serverblock;
		std::set<std::string>		unique_headers;
		int							content_length_flag;
		int							transfer_encoding_flag;
		int							chunked_flag;
		std::map<int, std::string>	status_message;
		bool						is_redirect;
		std::string					URI;
		bool						autoindex;
		std::string					boundary;
		std::string					content_type;
	public:
		HttpRequest();
		~HttpRequest();
		int 				get_request_line(std::fstream *file);
		int 				get_headers(std::fstream *file);
		int 				get_body(std::fstream *inFile, std::fstream *OutFile);
		int  				parse_request(std::fstream *file, std::list<struct ServerConfig> &serverList, int socket_fd);
		int  				parse_request_line(std::fstream *file);
		int  				parse_headers(std::fstream *file);
		int  				parse_body(std::fstream *inFile, std::fstream *OutFile);
		void				find_server_block(int socket_fd, std::list<struct ServerConfig> &serverList);
		std::string 		prepare_response(int status);
		void 				get_interface_port(int socket_fd);
		int					find_location();
		int					parse_method();
		int 				parse_http_version();
		bool				trim_and_transform(std::string &key, std::string &value);
		int					check_special_fields(std::string &key, std::string &value);
		std::string			handle_error_status(int status);
		std::string			build_response(int status, std::string path);
		std::string 	 	handle_get_method();
		std::string  		handle_post_method();
		std::string  		handle_delete_method();
		std::string 		handle_redirect();
		int 				build_URI_and_parse();
		void 				replace_string(std :: string &line, std :: string s1, std :: string s2);
		int 				is_directory();
		bool 				find_longest_match(int i, size_t &pos_match);
		std::string 		handle_directory_listing();
		int  				parse_path();
		bool				HasValidExtension(const std::string& path);
		std::string 		get_content_type(std::string path);
		std::string 		default_error_page();
		// add by moiz_sama to access data of struct for cgi
		std::string read_cgi_result(CgiHandler &cgi);
		std::string buildHttpResponse_cgi(const ParsedCgiOutput& parsed);
		// ===== GETTER METHODS =====
		// Basic request information
		const std::string &getMethod() const { return requestData.method; }
		const std::string &getPath() const { return requestData.path; }
		const std::string &getQueryString() const { return requestData.query; }
		const std::string &getHttpVersion() const { return requestData.version; }
		const std::string &getBody() const { return requestData.body; }
		const int &getPort() const { return port; }
		const std::string &getURI() const { return URI; }
		// Headers access
		const std::map<std::string, std::string> &getHeaders() const { return requestData.headers; }


		// Configuration getters
		const locationconfig &getLocationConfig() const { return location; }
		const serverconfig &getServerConfig() const { return serverblock; }
		int	getTransferEncodingFlag() const;
		int	getContentLength() const;
		bool	getBodyFlag() const { return body; }
		void	decrementContentLength(int amount) { content_length -= amount; }
		std::string	getBoundary() const { return boundary; }
		std::string	getContentType() const { return content_type; }


		class ServerBlockNotFoundException : std::exception
		{
			public:
			const char  *what() const throw();
		};
};
enum parse_status {
	PARSE_OK,
	ERROR_400,
	ERROR_401,
	ERROR_403,
	ERROR_404,
	ERROR_405,
	ERROR_408,
	ERROR_409,
	ERROR_411,
	ERROR_413,
	ERROR_414,
	ERROR_415,
	ERROR_422,
	ERROR_429,
	ERROR_500,
	ERROR_501,
	ERROR_502,
	ERROR_503,
	ERROR_504,
	ERROR_505,
	ERROR_508
};

#endif

