#include "../../includes/Cgi.hpp"

std::string CgiHandler::create_temp_file() {
    // Create unique temp file name using process ID and timestamp
    std::ostringstream temp_name;
    temp_name << "/tmp/webserv_body_" << getpid() << "_" << time(NULL) << ".txt";
    _temp_body_file = temp_name.str();
    return _temp_body_file;
}

void CgiHandler::cleanup_temp_file() {
    if (!_temp_body_file.empty() && access(_temp_body_file.c_str(), F_OK) == 0) {
        unlink(_temp_body_file.c_str());
        _temp_body_file.clear();
    }
}

void CgiHandler::setupPostData()
{
    if (!client_datafile) {
        // std::cerr << "[CHILD] No data file available" << std::endl;
        return;
    }
    
    // Seek to beginning of file
    client_datafile->clear();
    client_datafile->seekg(0, std::ios::beg);
    
    // Check if file has content
    client_datafile->seekg(0, std::ios::end);
    std::streampos file_size = client_datafile->tellg();
    if (file_size <= 0) {
        return; // Empty body
    }
    client_datafile->seekg(0, std::ios::beg);
    
    // Create temp file
    std::string temp_file = create_temp_file();
    if (temp_file.empty()) {
        // std::cerr << "[CHILD] Failed to create temp file for POST data" << std::endl;
        return;
    }
    
    int body_fd = open(temp_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (body_fd < 0) {
        // std::cerr << "[CHILD] Failed to create POST data file: " << strerror(errno) << std::endl;
        return;
    }
    
    // Read from fstream and write to temp file
    char buffer[4096];
    ssize_t total_written = 0;
    
    while (client_datafile->read(buffer, sizeof(buffer)) || client_datafile->gcount() > 0) {
        std::streamsize bytes_read = client_datafile->gcount();
        ssize_t written = write(body_fd, buffer, bytes_read);
        
        if (written != bytes_read) {
            // std::cerr << "[CHILD] Failed to write complete POST data" << std::endl;
            close(body_fd);
            unlink(temp_file.c_str());
            return;
        }
        total_written += written;
    }
    
    close(body_fd);
    
    if (total_written == 0) {
        unlink(temp_file.c_str());
        return;
    }
    
    // Reopen for reading and redirect to stdin
    body_fd = open(temp_file.c_str(), O_RDONLY);
    if (body_fd >= 0) {
        if (dup2(body_fd, STDIN_FILENO) == -1) {
            // std::cerr << "[CHILD] Failed to redirect stdin: " << strerror(errno) << std::endl;
        }
        close(body_fd);
    }
    
    // Clean up temp file (stays available via stdin fd)
    unlink(temp_file.c_str());
}

static std::string getDirname(const std::string &path) {
    size_t pos = path.find_last_of('/');
    if (pos == std::string::npos)
        return "."; // no slash → current directory
    if (pos == 0)
        return "/"; // root
    return path.substr(0, pos); // everything before last '/'
}


void CgiHandler::setupWorkingDirectory()
{
    std::string dir = getDirname(_full_script_Path);
    // std::cerr << "[CHILD] try to change directory to: " << dir << std::endl ;
    if (chdir(dir.c_str()) != 0) {
        throw std::runtime_error("[CHILD] Failed to change directory");
    }
}
