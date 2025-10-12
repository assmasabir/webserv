#include "../../includes/HttpRequest.hpp"


std::string HttpRequest::handle_redirect()
{

    std::ostringstream  response_stream;
    std::string  response;
    
    if(location.path == location.redirect)
        return(handle_error_status(ERROR_508));
    
    response_stream << requestData.version << " " << 303 << " " << "Found" <<"\r\n"
    << "Content-Type: text/html\r\n"
    << "Connection: close" <<"\r\n"
    << "Location: " << location.redirect << "\r\n"
    << "\r\n";
    response = response_stream.str();
    return response;
}

int HttpRequest::is_directory()
{
    if(!location.default_file.empty())
    {
        if(URI[URI.length() - 1] != '/')
            URI += "/";
        URI += location.default_file;
        if(access(URI.c_str(), R_OK))
            return ERROR_404;
    }
    else
    {
        if(location.directory_listing)
            autoindex = true;
        else
            return ERROR_403;
    }
    return PARSE_OK;
}

int HttpRequest::build_URI_and_parse()
{
    std::string resource;
    struct stat info;

    resource = requestData.path.substr(location.path.length());
    URI = location.root + "/" + resource;
    replace_string(URI, "//", "/");
    // std::cout << "i am uri " << URI << std::endl;
    if(stat(URI.c_str(), &info) == -1)
    {
        return ERROR_404;
    }
    else 
    {
        if(S_ISDIR(info.st_mode))
        {
            return(is_directory());
        }
        else if(S_ISREG(info.st_mode))
        {
            if(access(URI.c_str(), R_OK))
                return ERROR_403;
        }
        else
            return ERROR_404;
    }
    return PARSE_OK;
}


std::string HttpRequest::handle_get_method()
{
    int status;

    autoindex = false;
    if(!location.redirect.empty())
        return(handle_redirect());
    if((status = build_URI_and_parse()) == PARSE_OK)
    {
        if(autoindex)
            return(handle_directory_listing());
        else
            return build_response(200, URI);
    }
    return(handle_error_status(status));
}