
#include "../../includes/HttpRequest.hpp"

int HttpRequest::get_request_line(std::fstream *file) //check if there is more 
{
    int j;

    std::string line;
    std::string value;
    size_t pos = 0;
    j = 0;

    std::getline(*file, line);
    std::stringstream s1(line);
    while(s1 >> value)
    {
        if(j == 0)
            requestData.method = value;
        else if(j == 1)
            requestData.path = value;
        else
            requestData.version = value;
        j++;
    }
     if(j != 3)
        return ERROR_400;
    else if (requestData.path.size() >  4000)
        return(ERROR_414);
    if((pos = requestData.path.find('?', pos)) != std::string :: npos)
    {
        requestData.query = requestData.path.substr(pos);
        requestData.path = requestData.path.substr(0, pos);
    }
        
    return PARSE_OK;
}

bool	HttpRequest:: find_longest_match(int i, size_t &best_len)
{
    if(!requestData.path.compare(0, serverblock.locations[i].path.size(), serverblock.locations[i].path) 
        && serverblock.locations[i].path.size() > best_len)
    {
        best_len = serverblock.locations[i].path.size();
        return true;
    }
    return false;
}


bool 	HttpRequest:: HasValidExtension(const std::string& path) {
    size_t pos = path.find_last_of('.');
    if (pos == std::string::npos) return false;
    
    std::string ext = path.substr(pos);
    return ext == ".py" || ext == ".php" || ext == ".sh";
}

int	HttpRequest:: find_location()
{
    bool found;
    size_t best_len;

    best_len = 0;
    found = false;
    if(HasValidExtension(requestData.path))
    {
        for(size_t i = 0; i != serverblock.locations.size() && !found; i++)
        {
            if(serverblock.locations[i].path == "/cgi-bin")
            {
                location = serverblock.locations[i];
                found = true;
            }
        }
        
    }
    else
    {
        for(size_t i = 0; i != serverblock.locations.size(); i++)
        {
            if(find_longest_match(i, best_len))
            {
                location = serverblock.locations[i];
                found = true;
            }
        }
    }
    if(!found)
        return(ERROR_404);
    return(PARSE_OK);
}


int HttpRequest::parse_method()
{
    bool found;

    found = false;
    for(size_t i = 0; i != location.allowed_methods.size(); i++)
    {
        if(requestData.method == location.allowed_methods[i])
            found = true;
    }
    if  (!found)
    {
        if(requestData.method == "GET"
            || requestData.method == "POST"
            || requestData.method == "DELETE")
                return(ERROR_405);
        else if ( requestData.method != "HEAD"
            && requestData.method != "CONNECT"
            && requestData.method != "PUT"
            && requestData.method != "OPTIONS"
            && requestData.method != "TRACE"
            && requestData.method != "PATCH")
                return(ERROR_400);
        else
            return(ERROR_501);
    }
    return(PARSE_OK);
}

int HttpRequest::parse_http_version()
{
    if(!(requestData.version == "HTTP/1.0" || requestData.version == "HTTP/1.1"))
        return(ERROR_505);
    return(PARSE_OK);
}

int  HttpRequest::parse_path()
{
    size_t pos;
    std:: string hexcoded;
    std:: string replace;
    if((pos = requestData.path.find_first_not_of("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-._~/%")) != std::string::npos)
        return ERROR_400;
    pos = 0;
    while((pos = requestData.path.find('%', pos)) != std::string::npos)
    {
        hexcoded = requestData.path.substr(pos + 1, 2);
        if(hexcoded.empty() || hexcoded.length() != 2 || hexcoded.find_first_not_of("0123456789abcdefABCDEF") != std::string::npos)
            return ERROR_400;
        std::stringstream hexStream(hexcoded);
        size_t toint = 0;
        hexStream >> std::hex >> toint;
        // std::cout << "to int " << toint << std::endl;
        if (hexStream.fail() || (toint > 127) || (toint >= 65 && toint <= 90) || (toint >= 97 && toint <= 122)
        || toint == 45  || toint == 95 || toint == 46 || toint == 126)
            return (ERROR_400);
        hexcoded = "%" + hexcoded;
        replace = std::string(1, static_cast<char>(toint));
        replace_string(requestData.path, hexcoded, replace);
        pos = pos+2;
    }
    replace_string(requestData.path, "/./", "/");
    if(requestData.path.find("/../") != std::string::npos)
        return ERROR_400;
    return PARSE_OK;
}

int  HttpRequest:: parse_request_line(std::fstream *file)
{
    int status;

    if  (
        ((status = get_request_line(file)) != PARSE_OK)
        || ((status = parse_path()) != PARSE_OK )
        || ((status = find_location()) != PARSE_OK)
        || ((status = parse_method()) != PARSE_OK)
        || ((status = parse_http_version()) != PARSE_OK)
        )
            return(status);
    return(PARSE_OK);
}


