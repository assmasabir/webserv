#include "../../includes/HttpRequest.hpp"

std::string HttpRequest::handle_delete_method()
{
    std::string resource;
    struct stat info;
    if(!location.redirect.empty())
        return(handle_redirect());
    resource = requestData.path.substr(location.path.length());
    URI = location.root + "/" + resource;
    replace_string(URI, "//", "/");
    if(stat(URI.c_str(), &info) == -1)
    {
        switch (errno)
        {
            case EACCES:
            case EPERM:
            case EROFS:
                return handle_error_status(ERROR_403);
            case ENOENT:
                return handle_error_status(ERROR_404);
            default :
                return handle_error_status(ERROR_500);
        }
    }
    else 
    {
        if(S_ISDIR(info.st_mode))
        {
            if(rmdir(URI.c_str()))
            {
                    // std::cout << "i am errno " <<errno << std::endl;

                switch (errno)
                {
                    case EACCES:
                    case EPERM:
                    case EROFS:
                        return handle_error_status(ERROR_403);
                    case ENOENT:
                        return handle_error_status(ERROR_404);
                    case ENOTEMPTY:
                        return handle_error_status(ERROR_409);
                    default :
                        return handle_error_status(ERROR_500);
                }

            }
            return build_response(200, "src/www/response_success/del.html");
        }
        else if(S_ISREG(info.st_mode))
        {
            if(unlink(URI.c_str()))
            {
                switch (errno)
                {
                    case EACCES:
                    case EPERM:
                    case EROFS:
                        return handle_error_status(ERROR_403);
                    case ENOENT:
                        return handle_error_status(ERROR_404);
                    default :
                        return handle_error_status(ERROR_500);
                }
            }
            
            return build_response(200, "src/www/response_success/del.html"); 
        }
        else
            return handle_error_status(ERROR_403);
    }
}