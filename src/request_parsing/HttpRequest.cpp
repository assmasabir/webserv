#include "../../includes/HttpRequest.hpp"

HttpRequest::~HttpRequest()
{

}
HttpRequest::HttpRequest()
{
    body = false;
    transfer_encoding_flag = 0;
    content_length_flag = 0;
    content_length = 0;
    chunked_flag = 0;
    unique_headers.insert("HOST");
    unique_headers.insert("CONNECTION");
    unique_headers.insert("CONTENT-LENGTH");
    unique_headers.insert("CONTENT-TYPE");
    unique_headers.insert("AUTHORIZATION");
    unique_headers.insert("TRANSFER-ENCODING");
    unique_headers.insert("USER-AGENT");
    unique_headers.insert("LOCATION");
    unique_headers.insert("REFERER");
    unique_headers.insert("ORIGIN");
    unique_headers.insert("DATE");
    unique_headers.insert("SERVER");
    unique_headers.insert("ETAG");
    unique_headers.insert("IF-MATCH");
    unique_headers.insert("IF-NONE-MATCH");
    unique_headers.insert("IF-MODIFIED-SINCE");
    unique_headers.insert("IF-UNMODIFIED-SINCE");

    status_message.insert(std::make_pair(400, "Bad Request"));
    status_message.insert(std::make_pair(401, "Unauthorized"));
    status_message.insert(std::make_pair(403, "Forbidden"));
    status_message.insert(std::make_pair(404, "Not Found"));
    status_message.insert(std::make_pair(405, "Method Not Allowed"));
    status_message.insert(std::make_pair(408, "Request Timeout"));
    status_message.insert(std::make_pair(409, "Conflict"));
    status_message.insert(std::make_pair(411, "Length Required"));
    status_message.insert(std::make_pair(413, "Payload Too Large"));
    status_message.insert(std::make_pair(414, "URI Too Long"));
    status_message.insert(std::make_pair(415, "Unsupported Media Type"));
    status_message.insert(std::make_pair(422, "Unprocessable Entity"));
    status_message.insert(std::make_pair(429, "Too Many Requests"));


    status_message.insert(std::make_pair(500, "Internal Server Error"));
    status_message.insert(std::make_pair(501, "Not Implemented"));
    status_message.insert(std::make_pair(502, "Bad Gateway"));
    status_message.insert(std::make_pair(503, "Service Unavailable"));
    status_message.insert(std::make_pair(504, "Gateway Timeout"));
}


void HttpRequest::get_interface_port(int socket_fd)
{
    struct sockaddr_in     local_addr;
    socklen_t               addrlen;

    addrlen = sizeof local_addr;
    getsockname(socket_fd, (struct sockaddr*)&local_addr, &addrlen);
    interface = inet_ntoa(local_addr.sin_addr);
    port = ntohs(local_addr.sin_port);
    // std :: cout << "interface: " << interface << "port: "<< port << std::endl; 
       
}


void HttpRequest::find_server_block(int socket_fd, std::list<struct ServerConfig> &serverList)
{
    std::list<struct ServerConfig>::iterator    it_lis;
	std::vector<std::pair<int, std::string> >::iterator        it_inter;
    bool                                        check;

    get_interface_port(socket_fd);
    check = false;
    for(it_lis = serverList.begin(); it_lis != serverList.end(); it_lis++)
    {
        for(it_inter = it_lis->interface_port.begin(); it_inter != it_lis->interface_port.end(); it_inter++)
        {
            if(port == it_inter->first && interface == it_inter->second)
            {
                serverblock = *it_lis;
                return;
            }
            else if(it_inter->second == "0.0.0.0" && port == it_inter->first && !check)
            {
                check = true;
                serverblock = *it_lis;
            }
        }
    }
    if(!check)
        throw ServerBlockNotFoundException();
    
}

int  HttpRequest:: parse_request(std::fstream *file, std::list<struct ServerConfig> &serverList, int socket_fd)
{
    int                 status;

	file->flush();
    file->seekg(0);
    find_server_block(socket_fd, serverList);
    if((status =  parse_request_line(file)) != PARSE_OK
    || (status =  parse_headers(file)) != PARSE_OK)
    {
        return (status);
    }
    return PARSE_OK;
}

int	HttpRequest :: getContentLength() const
{
	return content_length;
}

int	HttpRequest :: getTransferEncodingFlag() const
{
	return transfer_encoding_flag;
}

const char  *HttpRequest::ServerBlockNotFoundException::what() const throw()
{
  return ("No server block found");
}
