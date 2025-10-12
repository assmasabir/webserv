#include "../../includes/Cgi.hpp"

bool CgiHandler::isValidScriptPath(const std::string& scriptPath) {
    // Prevent directory traversal
    if (scriptPath.find("..") != std::string::npos) return false;
    if (scriptPath.find("/etc") == 0) return false;  // Block system dirs
    // Must be in allowed script directories
    return scriptPath.find("/cgi-bin/") == 0;
}

std::vector<struct LocationConfig> CgiHandler::find_cgi_location(serverconfig _server)
{
    std::vector<LocationConfig> locations;
    
    // Find all locations that have CGI configuration
    for (size_t i = 0; i < _server.locations.size(); i++) {
        if (!_server.locations[i].cgi.empty()) {
            locations.push_back(_server.locations[i]);
        }
    }
    if (locations.empty()) {
        throw std::runtime_error("[CGI] No CGI-enabled locations found in server config");
    } 
    return locations;
};

LocationConfig* CgiHandler::getMatchingLocation(const std::string& path)
{
    LocationConfig* bestMatch = NULL;
    size_t longestMatchLength = 0;

    for (size_t i = 0; i < _cgi_location.size(); i++) {
        const std::string& locationPath = _cgi_location[i].path;
        
        if (path.find(locationPath) == 0 && locationPath.length() > longestMatchLength) {
            bestMatch = &_cgi_location[i];
            longestMatchLength = locationPath.length();
        }
    }
    return bestMatch;
}

bool CgiHandler::is_Cgi_Exist_in_location(std::string& path)
{       
    if (!__matched_location) {
        return false;
    }
    const std::map<std::string, std::string>& cgi_map = __matched_location->cgi;
    size_t dot_pos = path.find_last_of('.');
    if (dot_pos == std::string::npos) {
        return false;
    }
    std::string file_extension = path.substr(dot_pos);
    for (std::map<std::string, std::string>::const_iterator it = cgi_map.begin(); 
         it != cgi_map.end(); ++it) {
        const std::string& config_extension = it->first;
        const std::string& interpreter = it->second;
        if (file_extension == config_extension) {
            _cgiPath = interpreter;
            return true;
        }
    }   
    return false;
}

void Replace_string(std :: string &line, std :: string s1, std :: string s2)
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

void CgiHandler::initial_vars()
{
    try {
		// Get basic request info
		_method         = _req.getMethod();
		if (_method.empty())
			return throw std::runtime_error("[CGI] Request method is empty");
		
		_scriptFile     = _req.getPath();
		if (_scriptFile.empty())
			return throw std::runtime_error("[CGI] No script path found in request");

		//get config info mybe this work
		_srv_info = _req.getServerConfig();
        _cgi_location = find_cgi_location(_srv_info);
		if (_cgi_location.empty()) {
            return throw std::runtime_error("[CGI] No CGI locations found in server config");
        }

		// Find ahssan location for this script
        LocationConfig* location = getMatchingLocation(_scriptFile);
        if (!location) {
            return throw std::runtime_error("[CGI] No matching CGI location found for script");
        }
		setCurrentLocation(location);
		_root_path     = location->root; // /src/www/cgi-bin
        _location_path = location->path; // /cgi-bin
		
		// chech if type script file is in location
		_CGI_EXIST = is_Cgi_Exist_in_location(_scriptFile);
        if (_CGI_EXIST == false) {
            return throw std::runtime_error("[CGI] File is not a CGI script");
        }
		if (_cgiPath.empty()) {
            return throw std::runtime_error("[CGI] No CGI executable found for script type");
        }

        std::string relative_path = _scriptFile;
        if (relative_path.find(_location_path) == 0) {
            relative_path = relative_path.substr(_location_path.length());
        }
        if (!relative_path.empty() && relative_path[0] == '/') {
            relative_path = relative_path.substr(1);
        }
        _full_script_Path = _root_path;
        if (!_full_script_Path.empty() && _full_script_Path[_full_script_Path.length()-1] != '/') {
            _full_script_Path += "/";
        }
        _full_script_Path += relative_path;
        
		_CGI_OK = true;
        return ;
	} 
	
	catch (const std::exception& e)
	{
        _CGI_OK = false;
        _CGI_EXIST = false;
        throw;
    }
    
};

static std::string normalizeHeaderName(const std::string& name) {
    std::string out = name;
    // Uppercase
    std::transform(out.begin(), out.end(), out.begin(), ::toupper);
    // Replace '-' with '_'
    std::replace(out.begin(), out.end(), '-', '_');
    return out;
}

void CgiHandler::SetCgiEnvironment()
{
    const std::map<std::string, std::string>& http_headers = _req.getHeaders();
    
    // Basic CGI information
    env.Add("GATEWAY_INTERFACE", "CGI/1.1");
    env.Add("REQUEST_METHOD", _method);
    env.Add("SERVER_PROTOCOL", _req.getHttpVersion());
    env.Add("SERVER_SOFTWARE", "webserv/1.0");
    
    // Script information
    env.Add("SCRIPT_NAME", _req.getPath());
    env.Add("SCRIPT_FILENAME", _full_script_Path);
    env.Add("DOCUMENT_ROOT", _root_path);
    
    // Server information 
	const int port = _req.getPort();
	std::stringstream ss;
	ss << port;
	std::string str = ss.str();
    env.Add("SERVER_NAME", "localhost");
    env.Add("SERVER_PORT", str);
    
    // QUERY_STRING: everything after '?' in URL
    env.Add("QUERY_STRING", _req.getQueryString());
    
    // Content-Length
    std::map<std::string, std::string>::const_iterator cl_it = http_headers.find("CONTENT-LENGTH");
    if (cl_it != http_headers.end()) {
        env.Add("CONTENT_LENGTH", cl_it->second);
    } else {
         std::stringstream ss;
        ss << _req.getContentLength();
        env.Add("CONTENT_LENGTH", ss.str() );
    }

    for (std::map<std::string, std::string>::const_iterator it = http_headers.begin();
         it != http_headers.end(); ++it) {
        // Skip headers we've already processed
        if (it->first == "CONTENT-LENGTH") {
            continue;
        }
        if (it->first == "COOKIE" ){
            env.Add("HTTP_" + it->first, it->second);
            continue;
        }
        std::string key = normalizeHeaderName(it->first);    
        if (!key.empty()) {
            env.Add("HTTP_" + key, it->second);
        }
    }
}
