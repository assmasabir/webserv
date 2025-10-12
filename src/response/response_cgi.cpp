#include "../../includes/HttpRequest.hpp"
#include "../../includes/Cgi.hpp"
#include "../../includes/parse_output.hpp"

inline const char* to_string(CgiOutputType t) {
    switch (t) {
        case CGI_DOCUMENT_RESPONSE: return "DOCUMENT_RESPONSE";
        case CGI_LOCAL_REDIRECT:    return "LOCAL_REDIRECT";
        case CGI_CLIENT_REDIRECT:   return "CLIENT_REDIRECT";
        case CGI_NPH_RESPONSE:      return "NPH_RESPONSE";
        case CGI_STATUS_RESPONSE:   return "STATUS_RESPONSE";
        case CGI_PLAIN_TEXT:        return "PLAIN_TEXT";
        default:                    return "UNKNOWN";
    }
}

inline std::string ft_print(const ParsedCgiOutput& out) {
    std::ostringstream ss;
    ss << "ParsedCgiOutput {\n";
    ss << "  type          : " << to_string(out.type) << "\n";
    ss << "  valid         : " << (out.valid ? "true" : "false") << "\n";
    ss << "  status        : " << out.status_code << " " << out.status_message << "\n";

    ss << "  headers       :\n";
    for (std::map<std::string,std::string>::const_iterator it = out.headers.begin();
         it != out.headers.end(); ++it) {
        ss << "    " << it->first << ": " << it->second << "\n";
    }

    ss << "  cookies       :\n";
    for (size_t i=0; i<out.cookies.size(); ++i) {
        ss << "    Set-Cookie: " << out.cookies[i] << "\n";
    }

    ss << "  body          : " 
       << (out.body.empty() ? "(empty)" : out.body) << "\n";
    ss << "}\n";
    return ss.str();
}

std::string HttpRequest::buildHttpResponse_cgi(const ParsedCgiOutput& parsed)
{
    // std::cout << ft_print(parsed);
    std::ostringstream response;
    
    if (parsed.type == CGI_NPH_RESPONSE) {
        // CGI provided complete HTTP response
        response << requestData.version << " " << parsed.status_code << " " 
                 << parsed.status_message << "\r\n";
    } else {
        // Standard CGI response - we provide HTTP status line
        response << requestData.version << " " << parsed.status_code << " " 
                 << parsed.status_message << "\r\n";
    }
    
    // Add headers
    for (std::map<std::string, std::string>::const_iterator it = parsed.headers.begin();
         it != parsed.headers.end(); ++it) {
        response << it->first << ": " << it->second << "\r\n";
    }
    
    // Add cookies
    for (std::vector<std::string>::const_iterator it = parsed.cookies.begin();
         it != parsed.cookies.end(); ++it) {
        response << "Set-Cookie: " << *it << "\r\n";
    }
    
    // Add Content-Length if not present and we have a body
    if (parsed.headers.find("Content-Length") == parsed.headers.end() &&
        parsed.headers.find("content-length") == parsed.headers.end() &&
        !parsed.body.empty()) {
        response << "Content-Length: " << parsed.body.size() << "\r\n";
    }
    
    // Always add Connection header
    response << "Connection: close\r\n";
    
    // End headers
    response << "\r\n";
    
    // Add body
    response << parsed.body;
    
    return response.str();
}

bool hasValidExtension(const std::string& path) {
    size_t pos = path.find_last_of('.');
    if (pos == std::string::npos) return false;
    
    std::string ext = path.substr(pos);
    return ext == ".py" || ext == ".php" || ext == ".sh";
}

std::string HttpRequest::read_cgi_result(CgiHandler &cgi)
{
    int tmp_fd = cgi.getCgiTmpFd();
    std::string tmp_filename = cgi.getCgiTmpFilename();

{
    if (tmp_fd < 0) {
        std::cerr << "[ERROR] Invalid temp file descriptor" << std::endl;
        close(tmp_fd);
        return handle_error_status(ERROR_500);
    }

    if (tmp_filename.empty()) {
        std::cerr << "[ERROR] Empty temp filename" << std::endl;
        return handle_error_status(ERROR_500);
    }

    // Seek to beginning of temp file
    if (lseek(tmp_fd, 0, SEEK_SET) == -1) {
        // std::cerr << "[ERROR] Failed to seek temp file: " << strerror(errno) << std::endl;
        return handle_error_status(ERROR_500);
    }
}

    // Read the CGI output with size limits
    std::string cgi_output;
    char buffer[4096];
    ssize_t bytes_read;
    size_t total_read = 0;
    const size_t MAX_RESPONSE_SIZE = 10 * 1024 * 1024; // 10MB limit

    while ((bytes_read = read(tmp_fd, buffer, sizeof(buffer))) > 0) {
        if (total_read + bytes_read > MAX_RESPONSE_SIZE) {
            // std::cerr << "[ERROR] CGI response too large" << std::endl;
            return handle_error_status(ERROR_500);
        }
        cgi_output.append(buffer, bytes_read);
        total_read += bytes_read;
    }

    if (bytes_read < 0) {
        return handle_error_status(ERROR_500);
    }

    // Parse the CGI output
    ParsedCgiOutput parsed = CgiOutputParser::parser(cgi_output);
    if (!parsed.valid) {
        std::cerr << "[ERROR] Invalid CGI output format" << std::endl;
        return handle_error_status(ERROR_502); // Bad Gateway
    }
    
    if (!cgi.getCgiTmpFilename().empty() && access(cgi.getCgiTmpFilename().c_str(), F_OK) == 0) {
        unlink(cgi.getCgiTmpFilename().c_str());
        cgi.getCgiTmpFilename().clear();
    }
    return (buildHttpResponse_cgi(parsed));
}
