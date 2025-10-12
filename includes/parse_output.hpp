#ifndef PARSE_OUTPUT_HPP
#define PARSE_OUTPUT_HPP

#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctime>
#include <cstring>
#include <algorithm>
#include <cctype>
#include <unistd.h>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <vector>
#include <map>

enum CgiOutputType {
    CGI_DOCUMENT_RESPONSE,      // Headers + body
    CGI_LOCAL_REDIRECT,         // Location: /path
    CGI_CLIENT_REDIRECT,        // Location: http://...
    CGI_NPH_RESPONSE,          // Non-Parsed Headers - Complete HTTP response
    CGI_STATUS_RESPONSE,       // Status: 404 Not Found
    CGI_PLAIN_TEXT             // No headers, just content
};

struct ParsedCgiOutput {
    CgiOutputType type;
    int status_code;
    std::string status_message;
    std::map<std::string, std::string> headers;
    std::vector<std::string> cookies;  // Multiple Set-Cookie headers
    std::string body;
    bool valid;
    
    ParsedCgiOutput() : type(CGI_DOCUMENT_RESPONSE), status_code(200), 
                       status_message("OK"), valid(true) {}
};

static std::string trim(const std::string& str) {
        size_t start = str.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        
        size_t end = str.find_last_not_of(" \t\r\n");
        return str.substr(start, end - start + 1);
}

class CgiOutputParser
{
    public  :
        static ParsedCgiOutput parser(const std::string& raw_output)
        {
                ParsedCgiOutput result;
                if (raw_output.empty()) {
                    // std::cout << "[DEBUG] CGI OUTPUT IS EMPTY " << std::endl;
                    result.status_code = 204; // No Content
                    result.valid = true;
                    return result;
                }
                // Find header-body separator use "\r\n or \n\n"
                size_t separator_pos = findHeaderBodySeparator(raw_output);
                if (separator_pos == std::string::npos) {
                    // std::cout << "[DEBUG] CGI OUTPUT ONLY TEXT " << std::endl;
                    return parsePlainContent(raw_output);
                }

                std::string headers_section;
                std::string body_section;

                if (separator_pos != std::string::npos)
                {
                    headers_section = raw_output.substr(0, separator_pos);
                    
                    // Check which separator was found to determine body start position
                    size_t body_start;
                    if (raw_output.substr(separator_pos, 4) == "\r\n\r\n") {
                        body_start = separator_pos + 4;  // Skip \r\n\r\n
                    } else {
                        body_start = separator_pos + 2;  // Skip \n\n
                    }
                    
                    if (body_start <= raw_output.size()) {
                        body_section = raw_output.substr(body_start);
                    }
                }
                
                // std::cout << "===============================" << std::endl;
                // std::cout << raw_output << std::endl;
                // std::cout << "===============================" << std::endl;

                // Check if first line is HTTP status line
                if (isHttpStatusLine(headers_section)) {
                    // std::cout << "[DEBUG] CGI OUTPUT HAVE STATUS LINE " << std::endl;
                    return parseNphResponse(headers_section, body_section);
                }
                    
                else {
                    // std::cout << "[DEBUG] CGI OUTPUT HAVE only HEADERS " << std::endl;
                    return parseCgiHeaders(headers_section, body_section);
                }
                return result;
        };
    
    private :
        static size_t findHeaderBodySeparator(const std::string& output)
        {
			size_t pos = output.find("\r\n\r\n");
			if (pos != std::string::npos) return pos;
			
			pos = output.find("\n\n");
			if (pos != std::string::npos) return pos;
			
			return std::string::npos;
        }

        static ParsedCgiOutput parsePlainContent(const std::string& content)
        {
            ParsedCgiOutput result;
            result.type = CGI_PLAIN_TEXT;
            result.body = content;
            result.headers["Content-Type"] = "text/plain";
            result.valid = true;
            return result;
        }

        static bool isHttpStatusLine(const std::string& headers) {
            return headers.length() >= 8 && headers.substr(0, 5) == "HTTP/";
        }

        static ParsedCgiOutput parseNphResponse(const std::string& headers, const std::string& body)
        {
            ParsedCgiOutput result;
            result.type = CGI_NPH_RESPONSE;
            result.body = body;
        
            std::vector<std::string> lines = splitLines(headers);
            if (lines.empty()) {
                result.valid = false;
                return result;
            }
        
            // Parse HTTP status line: "HTTP/1.1 200 OK"
            if (!parseStatusLine(lines[0], result)) {
                result.valid = false;
                return result;
            }
            // Parse remaining headers
            for (size_t i = 1; i < lines.size(); i++) {
                int status = parseHeaderLine(lines[i], result);
                if (status == 1) {
                    // std::cerr << "[ERROR] invalid header" << std::endl;
                    result.valid = false;
                    result.status_code = 502;
                    result.status_message = "Bad Gateway";
                    return result;
                }
            }
            
            // result.valid = true;
    
            return result;
        }

        static std::vector<std::string> splitLines(const std::string& text)
        {
            // std::cout << "[DEBUG] splitLines input: '" << text << "'" << std::endl;
            // std::cout << "[DEBUG] splitLines length: " << text.length() << std::endl;
            
            std::vector<std::string> lines;
            std::string::size_type start = 0;
            std::string::size_type pos = 0;

            while ((pos = text.find('\n', start)) != std::string::npos) {
                std::string line = text.substr(start, pos - start);
                if (!line.empty() && line[line.size() - 1] == '\r') {
                    line.erase(line.size() - 1);
                }
                // std::cout << "[DEBUG] Found line: '" << line << "'" << std::endl;
                lines.push_back(line);
                start = pos + 1;
            }
            
            // Don't forget the last line if there's no trailing newline
            if (start < text.length()) {
                std::string line = text.substr(start);
                if (!line.empty() && line[line.size() - 1] == '\r') {
                    line.erase(line.size() - 1);
                }
                // std::cout << "[DEBUG] Last line: '" << line << "'" << std::endl;
                lines.push_back(line);
            }

            // std::cout << "[DEBUG] Total lines found: " << lines.size() << std::endl;
            return lines;
        }

        static bool parseStatusLine(const std::string& line, ParsedCgiOutput& result) {
            // Parse: "HTTP/1.1 200 OK"
            std::istringstream iss(line);
            std::string protocol, status_str; // HTTP/1.1 and 200 
            
            if (!(iss >> protocol >> status_str)) return false;
            
            result.status_code = std::atoi(status_str.c_str());
            if (result.status_code < 100 || result.status_code > 599) return false; //valid codes are between 100 and 599 
            
            // Get reason phrase (rest of line)
            std::getline(iss, result.status_message);
            if (!result.status_message.empty() && result.status_message[0] == ' ') {
                result.status_message = result.status_message.substr(1); // ex : ok
            }
            
            return true;
        }
        
        static int parseHeaderLine(const std::string& line, ParsedCgiOutput& result) {
            if (line.empty()) return 1;
            
            size_t colon_pos = line.find(':');
            if (colon_pos == std::string::npos ) return 1;


            
            std::string name = trim(line.substr(0, colon_pos));
            std::string value = trim(line.substr(colon_pos + 1));
            // std::cout << "name : " << name << "value : " << value;
            
            // Validate header name
            if (!isValidHeaderName(name)) {
                // std::cerr << "[WARNING] Invalid header name: '" << name << "'" << std::endl;
                return 1;
            }
            
            // Validate header value
            if (!isValidHeaderValue(value)) {
                // std::cerr << "[WARNING] Invalid header value for '" << name << "': '" << value << "'" << std::endl;
                return 1;
            }

            // Convert to lowercase for comparison
            std::string name_lower = name;
            std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), ::tolower);
            // std::cout << "headers : " << name_lower << "\n";


            if (name_lower == "status") {
                parseStatusHeader(value, result);
            } else if (name_lower == "location") {
                result.headers[name] = value;
            } else if (name_lower == "set-cookie") {
                result.cookies.push_back(value);
            } else {
                size_t check_agian = value.find(':');
                if (check_agian  != std::string::npos ) return 1;
                result.headers[name] = value;
            }
            return 0;
        }
        
        static void parseStatusHeader(const std::string& value, ParsedCgiOutput& result) {
            // Parse "404 Not Found" or just "404"
            std::istringstream iss(value);
            std::string status_str;
            
            if (iss >> status_str) {
                result.status_code = std::atoi(status_str.c_str()); //404
                // Get mataba9a
                std::getline(iss, result.status_message);
                if (!result.status_message.empty() && result.status_message[0] == ' ') {
                    result.status_message = result.status_message.substr(1); // Not Found
                }
            }
        }

        static bool isValidHeaderName(const std::string& name) {
            if (name.empty()) return false;
            
            // Header names must start with alphanumeric
            if (!std::isalnum(name[0])) return false;
            
            // Valid characters: alphanumeric, hyphen
            for (size_t i = 0; i < name.size(); i++) {
                char c = name[i];
                if (!std::isalnum(c) && c != '-' && c != '_') {
                    return false;
                }
            }
            return true;
        }
        
        static bool isValidHeaderValue(const std::string& value) {
            // Value can contain most printable ASCII except control characters
            for (size_t i = 0; i < value.size(); i++) {
                unsigned char c = value[i];
                // Reject control characters except tab
                if (c < 32 && c != '\t') {
                    return false;
                }
                // Reject DEL character
                if (c == 127) {
                    return false;
                }
            }
            return true;
        }

        static ParsedCgiOutput parseCgiHeaders(const std::string& headers, const std::string& body)
        {
            // std::cerr << "[DEBUG] PARSE CGI HEADRERS " << std::endl;
            ParsedCgiOutput result;
            result.body = body;
            std::vector<std::string> lines = splitLines(headers);
            for (size_t i = 0; i < lines.size(); i++) {
                // std::cerr << "[DEBUG] PARSE LINEs " << std::endl;
                // std::cout << "line : " << lines[i] << "\n";
                int status = parseHeaderLine(lines[i], result);
                if (status == 1) {
                    // std::cerr << "[ERROR] invalid header" << std::endl;
                    result.valid = false;
                    result.status_code = 502;
                    result.status_message = "Bad Gateway";
                    return result;
                }
            }
            
            // Determine response type and set defaults
            determineResponseType(result);
            result.valid = true;
            return result;
        }

        static void determineResponseType(ParsedCgiOutput& result) {
        if (result.headers.find("Location") != result.headers.end()) {
            std::string location = result.headers["Location"];
            if (isAbsoluteUrl(location)) {
                result.type = CGI_CLIENT_REDIRECT;
                if (result.status_code == 200) result.status_code = 302;
            } else {
                result.type = CGI_LOCAL_REDIRECT;
                if (result.status_code == 200) result.status_code = 302;
            }
        } else if (result.status_code != 200) {
            result.type = CGI_STATUS_RESPONSE;
        } else {
            result.type = CGI_DOCUMENT_RESPONSE;
        }
        
        // Set default Content-Type
        if (result.headers.find("Content-Type") == result.headers.end() &&
            result.headers.find("content-type") == result.headers.end()) {
            result.headers["Content-Type"] = "text/html";
        }
        
        // Set default status message
        if (result.status_message.empty()) {
            result.status_message = getDefaultStatusMessage(result.status_code);
        }
    }
    
        static bool isAbsoluteUrl(const std::string& url) {
            return url.find("://") != std::string::npos;
        }
    
        static std::string getDefaultStatusMessage(int code) {
        switch (code) {
            // 2xx Success
            case 200: return "OK";
            case 201: return "Created";
            case 202: return "Accepted";
            case 204: return "No Content";
            
            // 3xx Redirection
            case 301: return "Moved Permanently";
            case 302: return "Found";
            case 304: return "Not Modified";
            
            // 4xx Client Error
            case 400: return "Bad Request";
            case 401: return "Unauthorized";
            case 403: return "Forbidden";
            case 404: return "Not Found";
            case 405: return "Method Not Allowed";
            case 408: return "Request Timeout";
            case 411: return "Length Required";
            case 413: return "Payload Too Large";
            case 414: return "URI Too Long";
            case 415: return "Unsupported Media Type";
            case 429: return "Too Many Requests";
            
            // 5xx Server Error
            case 500: return "Internal Server Error";
            case 501: return "Not Implemented";
            case 502: return "Bad Gateway";
            case 503: return "Service Unavailable";
            case 504: return "Gateway Timeout";  // This was missing!
            case 505: return "HTTP Version Not Supported";
            
            default: return "Unknown Status";
        }
    }

};




#endif