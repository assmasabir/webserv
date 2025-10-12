#include "../../includes/HttpRequest.hpp"
#include "../../includes/Cgi.hpp"
#include "../../includes/parse_output.hpp"

std::string HttpRequest::prepare_response(int status)
{
    if(status != PARSE_OK)
        return handle_error_status(status);
    if(requestData.method == "GET")
        return(handle_get_method());
    else if(requestData.method == "POST")
        return(handle_post_method());
    else if(requestData.method == "DELETE")
        return(handle_delete_method());
    return handle_error_status(ERROR_500);
}

std::string HttpRequest::build_response(int status, std::string path)
{
    std::string response;
    std::ostringstream  response_stream;
    // std::cout << "i am path " << path << std::endl;
    std::fstream ss(path.c_str());

    if(!ss.is_open())
    {
        std::cerr << "error opening a file" << std::endl;
        return(default_error_page());
    }
    std::string body((std::istreambuf_iterator<char>(ss)), std::istreambuf_iterator<char>());
    ss.close();
    response_stream << requestData.version << " " << status << " " << status_message[status] <<"\r\n"
                    << "Content-Type: "<< get_content_type(path) <<"\r\n"
                    << "Content-Length: " << body.size() << "\r\n"
                    << "Connection: close" <<"\r\n"
                    << "\r\n"
                    << body;
    response = response_stream.str();
    return response;
}

std::string HttpRequest::handle_error_status(int status)
{
    switch (status)
    {
        case ERROR_400:
            return (build_response(400, serverblock.error[400]));
        case ERROR_401:
            return (build_response(401, serverblock.error[401]));
        case ERROR_403:
            return (build_response(403, serverblock.error[403]));
        case ERROR_404:
            return (build_response(404, serverblock.error[404]));
        case ERROR_405:
            return (build_response(405, serverblock.error[405]));
        case ERROR_408:
            return (build_response(408, serverblock.error[408]));
        case ERROR_409:
            return (build_response(409, serverblock.error[409]));
        case ERROR_411:
            return (build_response(411, serverblock.error[411]));
        case ERROR_413:
            return (build_response(413, serverblock.error[413]));
        case ERROR_414:
            return (build_response(414, serverblock.error[414]));
        case ERROR_415:
            return (build_response(415, serverblock.error[415]));
        case ERROR_422:
            return (build_response(422, serverblock.error[422]));
        case ERROR_429:
            return (build_response(429, serverblock.error[429]));
        case ERROR_500:
            return (build_response(500, serverblock.error[500]));
        case ERROR_501:
            return (build_response(501, serverblock.error[501]));
        case ERROR_502:
            return (build_response(502, serverblock.error[502]));
        case ERROR_503:
            return (build_response(503, serverblock.error[503]));
        case ERROR_504:
            return (build_response(504, serverblock.error[504]));
        case ERROR_505:
            return (build_response(505, serverblock.error[505]));
        case ERROR_508:
            return (build_response(508, serverblock.error[508]));
    }
    return(NULL);
}

std::string HttpRequest :: default_error_page()
{
     std::string response;
    std::ostringstream  response_stream;
    std::ostringstream ss;
    std::string value;

    ss  << "<html > \n"
        <<  "<head>\n"
        <<  "<style>\n"
        <<  " body {\n"
        << "background-color: black;\n"
        << "color:rgb(146, 3, 3);\n"
        << " font-family: 'Gill Sans', 'Gill Sans MT', Calibri, 'Trebuchet MS', sans-serif;\n"
        << "text-align: center;\n"
        << "} h1 \n"
        << "{padding-top: 20%;}\n"
        << "</style> \n"
        <<  "</head>\n"
        <<  "<body>\n"
        <<  " <h1>500 Internal Server Error</h1>\n"
        <<  "</body>\n"
        <<  "</html>\n";
    value = ss.str();
    const char *body = value.c_str();
    response_stream << requestData.version << " " << 500 << " " << status_message[500] <<"\r\n"
                    << "Content-Type: text/html\r\n"
                    << "Content-Length: " << std::strlen(body) << "\r\n"
                    << "Connection: close" <<"\r\n"
                    << "\r\n"
                    << body;
    response = response_stream.str();
    return response;   
}