#include "../../includes/HttpRequest.hpp"


std::string HttpRequest::handle_post_method()
{
	return build_response(200, "src/www/response_success/post.html");
}
