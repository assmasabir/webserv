#include "../../includes/HttpRequest.hpp"

std::string HttpRequest::handle_directory_listing()
{
    std::string response;
    std::ostringstream  response_stream;
    std::ostringstream ss;
    std::string value;

    DIR *dir = opendir(URI.c_str());
    struct dirent *entry;
    if(!dir)
        return(handle_error_status(ERROR_404));
    ss  << "<html > \n"
        <<  "<head>\n"
        <<  "<title>Webserv</title>\n"
        <<  "<style>\n"
        << "body {\n"
        << "  display: flex;\n"
        << "  flex-direction: column;\n"
        << "  justify-content: center;\n"
        << "  align-items: center;\n"
        << "  background-color: black;\n"
        << "  height: 90vh;\n"
        << "  color: whitesmoke;\n"
        << "  font-family: 'Gill Sans', 'Gill Sans MT', Calibri, 'Trebuchet MS', sans-serif;\n"
        << "  text-align: center;\n"
        << "}\n"
        <<  "h1 {\n"
        <<  "}\n"
        <<  "a {\n"
        <<  "color: whitesmoke;\n"
        <<  "text-decoration: underline;\n"
        <<  "font-size: 18px;\n"
        <<  "}\n"
        <<  "a:hover {\n"
        <<  "color: lightblue;\n"
        <<  "}\n"
        <<  "</style>\n"
        <<  "</head>\n"
        <<  "<body>\n"
        <<  "<h1>Directory listing :</h1>\n";
    while ((entry = readdir(dir)) != NULL)
    {
        if(std::string(entry->d_name) == "." || std::string(entry->d_name) == "..")
            continue;
        value = location.path + "/" + entry->d_name;
        if(entry->d_type == DT_DIR)
            ss << "<a href=\"" << value << "/" <<"\"> " << entry->d_name << "/" << "</a><br>" ;
        else
            ss << "<a href=\"" << value <<"\"> " << entry->d_name << "</a><br>" ;
    }
    ss  <<  "</body>\n"
        <<  "</html>\n";
    value = ss.str();
    const char *body = value.c_str();
    response_stream << requestData.version << " " << 200 << " " << status_message[200] <<"\r\n"
                    << "Content-Type: text/html\r\n"
                    << "Content-Length: " << std::strlen(body) << "\r\n"
                    << "Connection: close" <<"\r\n"
                    << "\r\n"
                    << body;
    response = response_stream.str();
    closedir(dir);
    return response;
}


