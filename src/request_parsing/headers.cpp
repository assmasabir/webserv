#include "../../includes/HttpRequest.hpp"


bool HttpRequest::trim_and_transform(std::string &key, std::string &value)
{
    key.erase(0, key.find_first_not_of(" \t"));
    key.erase(key.find_last_not_of(" \t") + 1);
    value.erase(0, value.find_first_not_of(" \t"));
    value.erase(value.find_last_not_of(" \t") + 1);
    std::transform(key.begin(), key.end(), key.begin(), ::toupper);
    return true;
}

int HttpRequest::check_special_fields(std::string &key, std::string &value)
{
    if(key == "CONTENT-LENGTH")
    {
        content_length = std::atoi(value.c_str());
        if(value.find_first_not_of("0123456789") != std::string::npos || content_length < 0)
            return ERROR_400;
        if(content_length > 0)
            body = true;
	if ((size_t)content_length > serverblock.max_body_size)
		return (ERROR_413);
        content_length_flag = 1;
    }
    else if(key== "TRANSFER-ENCODING")
    {
        transfer_encoding_flag = 1;
        if(value == "chunked")
        {
            chunked_flag = 1;
            body = true;
        }
        else
            return ERROR_501;
    }
    if(key== "CONTENT-TYPE")
    {
        size_t  pos = value.find(";");
        if (pos == std::string::npos)
        {
            content_type = "NO_BOUNDARY";
            boundary = "NO_BOUNDARY";
            return (PARSE_OK);
        }
        std::string contentType = value.substr(0, pos);
        std::stringstream   ss;
        ss << contentType;
        ss >> contentType;
        std::string tmp;
        if (ss >> tmp)
            return ERROR_400;
        if (contentType != "multipart/form-data")
            return ERROR_501;
        std::string bound = value.substr(pos + 1);
        std::stringstream   ss2;
        ss2 << bound;
        ss2 >> bound;
        if (ss2 >> tmp)
            return ERROR_400;
        if (bound.compare(0, 13, "boundary=----"))
            return ERROR_400;
        boundary = "--" + bound.substr(9);
    }
    return PARSE_OK;
}

int HttpRequest::get_headers(std::fstream *file)
{
    std::string line;
    std::string value;
    std::string key;

    while(std::getline(*file, line) && !line.empty() && line != "\r")
    {
        int status;

        if(line[line.length() - 1] != '\r' || line.find(':') == std::string::npos)
            return(ERROR_400);
        std::stringstream s1(line);
        std::getline(s1, key, ':');
        std::getline(s1, value, '\r');
        if  (key.empty() 
        || value.empty()
        || !trim_and_transform(key, value)
        || key.find_first_not_of("!#$%&'*+-.^_`|~0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ") != std::string::npos
        || value.find_first_not_of("\t !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~") != std::string::npos
        || (requestData.headers.find(key) !=  requestData.headers.end() && unique_headers.find(key) != unique_headers.end())
        )
            return ERROR_400;
        else if ((status = check_special_fields(key, value)) != PARSE_OK)
            return (status);
        requestData.headers.insert(std::make_pair(key, value));
    }
    return PARSE_OK;
}



int HttpRequest::parse_headers(std::fstream *file)
{
    int status;
    if((status = get_headers(file)) != PARSE_OK)
        return(status);
    // std::cout << "started to parse headers\n";
    if(requestData.headers.find("HOST") == requestData.headers.end() && requestData.version == "HTTP/1.1")
        return(ERROR_400);
    if(body)
    {
        switch((content_length_flag + transfer_encoding_flag))
        {
            case 2:
                return ERROR_400;
            case 0:
                return ERROR_411;
        }
        if(boundary.empty())
            return ERROR_400;
    }
    
    return(PARSE_OK);
}
