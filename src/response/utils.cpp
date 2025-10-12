#include "../../includes/HttpRequest.hpp"


void HttpRequest:: replace_string(std :: string &line, std :: string s1, std :: string s2)
{
    size_t pos = 0;
    int s1_length;

    s1_length = s1.length();
    while((pos = line.find(s1, pos)) != std :: string :: npos)
    {
        line = line.substr(0, pos) + s2 + line.substr(pos + s1_length);
        pos += s2.length();
    }
}

std::string HttpRequest::get_content_type(std::string path)
{
    std::string extention;
    std::map<std::string, std::string>::iterator it;
    size_t pos;

    pos = path.rfind('.');
    if(pos != std::string::npos )
    {
        extention = path.substr(pos + 1);
        if(!serverblock.extension_path.empty() && ((it = serverblock.extension_path.find(extention)) != serverblock.extension_path.end()))
            return (it->second);
    }
    return("application/octet-stream");
}