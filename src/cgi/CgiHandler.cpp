#include "../../includes/Cgi.hpp"
#include "../../includes/webserv.hpp"
#include "../../includes/server.hpp"
#include "../../includes/HttpRequest.hpp"

CgiHandler::CgiHandler(HttpRequest &req) 
    : _req(req), 
      __matched_location(NULL),
      _temp_body_file(""),
      _CGI_OK(false), 
      _CGI_EXIST(false),
      _cgi_state(CGI_STATE_INIT),
      _cgi_pid(-1),
      _cgi_pipe_fd(-1),
      _cgi_tmp_fd(-1),
      _cgi_tmp_filename(""),
      _cgi_envp_built(false),
      _cgi_child_status(0),
      _cgi_start_time(time(NULL)),
      _total_read(0)
{
}


CgiHandler::~CgiHandler()
{
    cgiCleanup();
}

bool CgiHandler::handleCGIRequest()
{
    switch (_cgi_state) {
        case CGI_STATE_INIT:
            cgiInit();
            break;
        case CGI_STATE_SETUP_ENV:
            cgiSetupEnvironment();
            break;
        case CGI_STATE_FORK:
            cgiFork();
            break;
        case CGI_STATE_READ_OUTPUT:
            cgiReadOutput();
            break;
        case CGI_STATE_WAIT_CHILD:
            cgiWaitChild();
            break;
        case CGI_STATE_PROCESS_RESULT:
            cgiProcessResult();
            break;
        case CGI_STATE_CLEANUP:
            cgiCleanup();
            break;
        case CGI_STATE_DONE:
            return true;
        case CGI_STATE_ERROR:
            cgiCleanup();
            return true;
        default:
            // std::cerr << "[CGI ERROR] Invalid state: " << _cgi_state << std::endl;
            _cgi_state = CGI_STATE_ERROR;
            break;
    }
    return false;
}

void CgiHandler::cgiInit()
{
    try {
        // std::cout << "[CGI] State: INIT" << std::endl;
        initial_vars();
        if (_CGI_OK && _CGI_EXIST) {
            _cgi_state = CGI_STATE_SETUP_ENV;
        } else {
            // std::cerr << "[CGI] Initialization failed - not a valid CGI request" << std::endl;
            _cgi_state = CGI_STATE_ERROR;
        }
    } catch (const std::exception& e) {
        // std::cerr << "[CGI] Init Error: " << e.what() << std::endl;
        _cgi_state = CGI_STATE_ERROR;
    }
}

void CgiHandler::cgiSetupEnvironment()
{
    // std::cout << "[CGI] State: SETUP_ENV" << std::endl;   
    try {
        if (_cgi_envp_built) {
            for (size_t i = 0; i < _cgi_envp.size(); i++) {
                if (_cgi_envp[i]) {
                    free(_cgi_envp[i]);
                }
            }
            _cgi_envp.clear();
        }        
        SetCgiEnvironment();
        char** env_raw = env.GetRawEnv();
        for (int i = 0; env_raw[i] != NULL; i++) {
            _cgi_envp.push_back(strdup(env_raw[i]));
        }
        _cgi_envp.push_back(NULL);
        delete_strings(env_raw);       
        _cgi_envp_built = true;
        _cgi_state = CGI_STATE_FORK;
        
    } catch (const std::exception& e) {
        // std::cerr << "[CGI] Environment Setup Error: " << e.what() << std::endl;
        _cgi_state = CGI_STATE_ERROR;
    }
}

static std::string getFilename(const std::string &path) {
    size_t pos = path.find_last_of('/');
    if (pos == std::string::npos)
        return path; // no slash → whole string is filename
    return path.substr(pos + 1);
}

void CgiHandler::cgiFork()
{
    // std::cout << "[CGI] State: FORK : " << _body_fd << std::endl;
    int pipeOut[2];
    if (pipe2(pipeOut, O_CLOEXEC) == -1) {
        _cgi_state = CGI_STATE_ERROR;
        return;
    }  

    // Create temporary file for CGI output before fork
    {std::ostringstream temp_name;
    temp_name << "/tmp/cgi_response_" << getpid() << "_" << time(NULL) << ".tmp";
    _cgi_tmp_filename = temp_name.str();   
    _cgi_tmp_fd = open(_cgi_tmp_filename.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0600);
    if (_cgi_tmp_fd < 0) {
        // std::cerr << "[CGI] Failed to create temp file: " << strerror(errno) << std::endl;
        close(pipeOut[0]);
        close(pipeOut[1]);
        _cgi_state = CGI_STATE_ERROR;
        return;
    }}

    _total_read = 0;   
    _cgi_pid = fork();
    if (_cgi_pid == -1) {
        close(pipeOut[0]);
        close(pipeOut[1]);
        close(_cgi_tmp_fd);
        unlink(_cgi_tmp_filename.c_str());
        _cgi_state = CGI_STATE_ERROR;
        return;
    } 

    if (_cgi_pid == 0) {
        // std::cout << "[CHILD] Starting CGI execution" << std::endl;
        close(2);
        close(pipeOut[0]);
        close(_cgi_tmp_fd);
        try {
            // Setup stdin for POST data if needed
            if ( _method == "POST") {
                
                setupPostData();
            }

            if (dup2(pipeOut[1], STDOUT_FILENO) == -1) 
                _exit(1);
            // dup2(pipeOut[1], STDERR_FILENO);
            close(pipeOut[1]);
            setupWorkingDirectory();
            std::string script_arg = getFilename(_scriptFile);
            char* args[] = { const_cast<char*>(_cgiPath.c_str()), const_cast<char*>(script_arg.c_str()), NULL };
            execve(_cgiPath.c_str(), args, &_cgi_envp[0]);
            _exit(127);    
        } catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
            _exit(1);
        }   
        _exit(1);    
    }
    
    else {
        // std::cout << "[PARENT] Child PID: " << _cgi_pid << std::endl;
        close(pipeOut[1]);

        // Make pipe non-blocking for parent
        int flags = fcntl(pipeOut[0], F_GETFL, 0);
        if (flags != -1) {
            fcntl(pipeOut[0], F_SETFL, flags | O_NONBLOCK);
        }

        _cgi_pipe_fd = pipeOut[0];
        _cgi_state = CGI_STATE_READ_OUTPUT;
        // std::cout << "[PARENT] CGI setup complete, pipe_fd: " << _cgi_pipe_fd << std::endl;
    }
}

void CgiHandler::cgiReadOutput()
{
    // std::cout << "[CGI] State: CGI_READ_OUTPUT :" << std::endl;

    time_t current_time = time(NULL);
    if (current_time - _cgi_start_time > CGI_TIMEOUT_SECONDS) {
        // std::cerr << "[CGI] Timeout in READ_OUTPUT - killing process " << _cgi_pid << std::endl;
        if (_cgi_pid > 0) {
            kill(_cgi_pid, SIGKILL);
        }
        _cgi_state = CGI_STATE_ERROR;
        _cgi_child_status = 504;
        return;
    }

    char buffer[CGI_READ_BUFFER_SIZE];
    ssize_t bytesRead = read(_cgi_pipe_fd, buffer, sizeof(buffer));
    
    if (bytesRead > 0) {
        // std::cout << "[CGI] Read " << bytesRead << " bytes from CGI (total: " << (_total_read + bytesRead) << ")" << std::endl;
        
        // Check for output size limit
        if (_total_read + bytesRead > MAX_CGI_OUTPUT_SIZE) {
            std::cerr << "[CGI] Output size limit exceeded" << std::endl;
            _cgi_state = CGI_STATE_ERROR;
            return;
        }
        
        // Write to temporary file
        ssize_t written = write(_cgi_tmp_fd, buffer, bytesRead);
        if (written != bytesRead) {
            std::cerr << "[CGI] Failed to write all data to temp file: " << written << "/" << bytesRead << std::endl;
            _cgi_state = CGI_STATE_ERROR;
            return;
        }
        
        _total_read += bytesRead;
        // Stay in this state - poll will notify us when more data is available or pipe closes
        return;
        
    } else if (bytesRead == 0) {
        // EOF - CGI finished writing
        // std::cout << "[CGI] CGI finished writing output (total: " << _total_read << " bytes)" << std::endl;
        if (_cgi_pipe_fd != -1) {
            close(_cgi_pipe_fd);
            _cgi_pipe_fd = -1;
        }
        _cgi_state = CGI_STATE_WAIT_CHILD;
        
    } else if (bytesRead == -1) {
        
            _cgi_state = CGI_STATE_ERROR;
    }
}

void CgiHandler::cgiWaitChild()
{
    // std::cout << "[CGI] State: WAIT_CHILD" << std::endl;

    time_t current_time = time(NULL);
    if (current_time - _cgi_start_time > CGI_TIMEOUT_SECONDS) {
        // std::cerr << "[CGI] CGI execution timeout, killing process " << _cgi_pid << std::endl;
        // std::cout << "[CGI] Time after CGI Child :" << current_time - _cgi_start_time << std::endl;
        kill(_cgi_pid, SIGKILL);     
        _cgi_state = CGI_STATE_ERROR;
        _cgi_child_status = 504;
        return;
    }
       
    pid_t result = waitpid(_cgi_pid, &_cgi_child_status, WNOHANG);   
    if (result == 0) {
        // Child still running, check again later
        return;
    } else if (result > 0) {
        // Child finished
        // std::cout << "[CGI] Child process finished with status: " << _cgi_child_status << std::endl;
        _cgi_state = CGI_STATE_PROCESS_RESULT;
    } else {
        // Error in waitpid
        // std::cerr << "[CGI] waitpid error: " << strerror(errno) << std::endl;
        _cgi_state = CGI_STATE_ERROR;
    }
}

void CgiHandler::cgiProcessResult()
{
    // std::cout << "[CGI] State: PROCESS_RESULT" << std::endl;
    
    if (WIFEXITED(_cgi_child_status)) {
        int exit_status = WEXITSTATUS(_cgi_child_status);
        if (exit_status == 0) {
            // std::cout << "[CGI] CGI executed successfully" << std::endl;
            _cgi_state = CGI_STATE_CLEANUP;
        } else {
            // std::cerr << "[CGI] CGI failed with exit status: " << exit_status << std::endl;
            _cgi_state = CGI_STATE_ERROR;
        }
    } else if (WIFSIGNALED(_cgi_child_status)) {
        // int signal_num = WTERMSIG(_cgi_child_status);
        // std::cerr << "[CGI] CGI terminated by signal: " << signal_num << std::endl;
        _cgi_state = CGI_STATE_ERROR;
    } else {
        // std::cerr << "[CGI] CGI terminated abnormally" << std::endl;
        _cgi_state = CGI_STATE_ERROR;
    }
}

void CgiHandler::cgiCleanup()
{
    if (_cgi_state == CGI_STATE_DONE) {
        return;  // Already cleaned up
    }
    
    // std::cout << "[CGI] State: CLEANUP" << std::endl;
    
    // Clean up environment variables
    if (_cgi_envp_built) {
        for (size_t i = 0; i < _cgi_envp.size(); i++) {
            if (_cgi_envp[i]) {
                free(_cgi_envp[i]);
                _cgi_envp[i] = NULL;
            }
        }
        _cgi_envp.clear();
        _cgi_envp_built = false;
    }
    
    // Close pipe if still open
    if (_cgi_pipe_fd != -1) {
        close(_cgi_pipe_fd);
        _cgi_pipe_fd = -1;
    }
    
    // Kill child process if still running
    if (_cgi_pid > 0) {
        int status;
        pid_t result = waitpid(_cgi_pid, &status, WNOHANG);
        if (result == 0) {
            // Child still running, send SIGTERM then SIGKILL if needed
            // std::cout << "[CGI] Terminating CGI process " << _cgi_pid << std::endl;
            kill(_cgi_pid, SIGKILL);
        }
        _cgi_pid = -1;
    }
    
    // Clean up temporary body file
    cleanup_temp_file();
    
    // Clear environment
    env.clear();
    
    // Reset state
    _cgi_child_status = 0;
    
    if (_cgi_state != CGI_STATE_ERROR) {
        _cgi_state = CGI_STATE_DONE;
    }
}