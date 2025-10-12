#ifndef CGI_HPP
#define CGI_HPP

#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctime>
#include <cstring>
#include <algorithm>
#include <cctype>
#include "Environment.hpp"
#include "./HttpRequest.hpp"

static const size_t CGI_READ_BUFFER_SIZE = 4096;
static const size_t MAX_CGI_OUTPUT_SIZE = 10 * 1024 * 1024; // 10MB
static const int CGI_TIMEOUT_SECONDS = 10; // 10sec

enum CgiState
{
	CGI_STATE_INIT,
	CGI_STATE_SETUP_ENV,
	CGI_STATE_FORK,
	CGI_STATE_READ_OUTPUT,
	CGI_STATE_WAIT_CHILD,
	CGI_STATE_PROCESS_RESULT,
	CGI_STATE_CLEANUP,
	CGI_STATE_DONE,
	CGI_STATE_ERROR
};

class CgiHandler
{
private:
	Environment					env;
	std::string					_method;
	std::string					_cgiPath;
	std::string					_scriptFile;
	std::string					_location_path;
	std::string					_root_path;
	std::string					_full_script_Path;
	std::string					_body;
	int							_body_fd;
	HttpRequest					&_req;
	serverconfig				_srv_info;
	std::vector<struct LocationConfig> _cgi_location;
	LocationConfig				*__matched_location;
	std::string					_temp_body_file;
	bool						_CGI_OK;
	bool						_CGI_EXIST;
	int							client_fd;
	std::fstream 				*client_datafile;

	// State machine variables
	CgiState 					_cgi_state;
	pid_t 						_cgi_pid;
	int 						_cgi_pipe_fd;
	int 						_cgi_tmp_fd;
	std::string 				_cgi_tmp_filename;
	std::vector<char *> 		_cgi_envp;
	bool 						_cgi_envp_built;
	int 						_cgi_child_status;
	time_t 						_cgi_start_time;
	size_t 						_total_read;

public:
	// Constructor/Destructor
	CgiHandler(HttpRequest &req);
	~CgiHandler();

	// Core interface methods
	bool 		handleCGIRequest();

	// State machine methods
	void 		cgiInit();
	void 		cgiSetupEnvironment();
	void 		cgiFork();
	void 		cgiReadOutput();
	void 		cgiWaitChild();
	void 		cgiProcessResult();
	void 		cgiCleanup();

	// Helper methods
	void 		setupPostData();
	void 		setupWorkingDirectory();
	void 		initial_vars();
	void 		SetCgiEnvironment();
	bool 		is_Cgi_Exist_in_location(std::string &path);
	std::vector<struct LocationConfig> find_cgi_location(serverconfig _server);
	bool 		isValidScriptPath(const std::string &scriptPath);
	std::string create_temp_file();
	void 		cleanup_temp_file();
	LocationConfig *getMatchingLocation(const std::string &path);

	// State getters
	bool 		isCgiDone() const { return _cgi_state == CGI_STATE_DONE || _cgi_state == CGI_STATE_ERROR; }
	bool 		isCgiError() const { return _cgi_state == CGI_STATE_ERROR; }
	CgiState 	getCgiState() const { return _cgi_state; }
	
	// Process info getters
	pid_t 		getCgiPid() const { return _cgi_pid; }
	time_t 		getCgiStartTime() const { return _cgi_start_time; }
	int 		getCgiChildStatus() const { return _cgi_child_status; }
	
	// File descriptor getters
	int 		getCgiPipeFd() const { return _cgi_pipe_fd; }
	int 		getCgiTmpFd() const { return _cgi_tmp_fd; }
	std::string getCgiTmpFilename() const { return _cgi_tmp_filename; }
	
	// Path getters
	const std::string& getCgiPath() const { return _cgiPath; }
	const std::string& getScriptFile() const { return _scriptFile; }
	const std::string& getRootPath() const { return _root_path; }
	const std::string& getLocationPath() const { return _location_path; }
	std::string getFullScriptPath() const { return _root_path + _scriptFile; }
	
	// Request data getters
	const std::string& getMethod() const { return _method; }
	const std::string& getBody() const { return _body; }
	size_t 	getBodySize() const { return _body.size(); }
	bool 	hasBody() const { return !_body.empty(); }
	size_t 	getTotalRead() const { return _total_read; }
	
	// Status getters/setters
	bool 	isCgiOk() const { return _CGI_OK; }
	bool 	isCgi() const { return _CGI_EXIST; }
	void 	setCgiOk(bool status) { _CGI_OK = status; }
	void 	setIsCgi(bool status) { _CGI_EXIST = status; }
	void 	setCgiError() { _cgi_state = CGI_STATE_ERROR; }
	void 	setCgiStatus(int status) { _cgi_child_status = status; }
	void 	setClientFd(int fd) { client_fd = fd; }
	void 	setDataClient(std::fstream 		*client_data) { client_datafile = client_data; }
	
	// Configuration getters/setters
	const serverconfig& getServerConfig() const { return _srv_info; }
	const std::vector<struct LocationConfig>& getCgiLocations() const { return _cgi_location; }
	LocationConfig* getCurrentLocation() const { return __matched_location; }
	void 	setCurrentLocation(LocationConfig *location) { __matched_location = location; }
	
	// Environment getters
	const Environment& getEnvironment() const { return env; }
	bool 	isEnvpBuilt() const { return _cgi_envp_built; }
};

#endif
