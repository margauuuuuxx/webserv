# Configuration Files Guide

This directory contains various configuration files for testing the webserv project.

## Configuration Files

### 1. **simplest_config.conf** (Recommended for initial testing)
A clean, minimal configuration that includes all basic features:
- Single server on port 8080
- Static file serving
- File upload/delete functionality
- Python and Bash CGI support
- Directory autoindex

**Use case:** Quick testing, development, debugging
```bash
./webserv conf_examples/simplest_config.conf
```

---

### 2. **test_basic.conf** (Basic functionality testing)
Tests fundamental HTTP server operations:
- ✓ GET, POST, DELETE methods
- ✓ Static file serving
- ✓ Error page handling (404, 405, 500)
- ✓ File upload with size limits (1MB)
- ✓ Autoindex on/off
- ✓ Multiple routes with different permissions

**Use case:** Validate basic HTTP server functionality
```bash
./webserv conf_examples/test_basic.conf
```

**Test scenarios:**
- Access http://localhost:8080/ (GET index.html)
- Upload files to http://localhost:8080/upload (POST)
- Delete files at http://localhost:8080/uploads/ (DELETE)
- Test 404 errors with non-existent paths
- Test autoindex at http://localhost:8080/hello

---

### 3. **test_cgi.conf** (CGI functionality testing)
Focuses on Common Gateway Interface execution:
- ✓ Python CGI scripts (.py)
- ✓ Bash CGI scripts (.sh)
- ✓ GET with query strings
- ✓ POST data to CGI
- ✓ Environment variables (SERVER_*, REQUEST_*, etc.)
- ✓ CGI timeout handling (504 errors)
- ✓ Large upload support (5MB)

**Use case:** Validate CGI implementation and script execution
```bash
./webserv conf_examples/test_cgi.conf
```

**Test scenarios:**
- Execute http://localhost:8080/python-cgi/test.py
- Execute http://localhost:8080/bash-cgi/test.sh
- Send POST data to CGI scripts
- Test query strings: /python-cgi/test.py?param=value
- Upload files and process with CGI

---

### 4. **test_advanced.conf** (Advanced features testing)
Comprehensive testing with multiple servers and edge cases:
- ✓ Multiple servers on different ports (8080, 8081, 8082)
- ✓ Virtual hosting (server_name)
- ✓ Various body size limits (100 bytes, 2MB, 10MB)
- ✓ Method restrictions per route
- ✓ Complex routing scenarios
- ✓ Stress testing with minimal configuration

**Use case:** Comprehensive testing, edge cases, multi-server setup
```bash
./webserv conf_examples/test_advanced.conf
```

**Test scenarios:**
- Main server: http://localhost:8080/
- API server: http://localhost:8081/
- Minimal server: http://localhost:8082/
- Test body size limits (try uploading 200 bytes to port 8082)
- Test method restrictions (POST to GET-only routes)
- Test virtual hosting with different server_name values

---

### 5. **config.conf** (Full featured example)
Demonstrates all available configuration directives:
- Multiple server blocks
- All error page types
- Complete route configurations
- All supported methods
- CGI configurations

**Use case:** Reference implementation, production-like setup
```bash
./webserv conf_examples/config.conf
```

---

### 6. **tester.conf** (Legacy/specific tester compatibility)
Original configuration for specific test scenarios.

**Use case:** Compatibility with external testers
```bash
./webserv conf_examples/tester.conf
```

---

## Quick Reference

### Configuration Directives

**Server Block:**
- `listen <port>` - Port to listen on
- `host <ip>` - Host IP address
- `server_name <names>` - Server names for virtual hosting
- `error_page <code> <path>` - Custom error pages
- `clientMaxBodySize <bytes>` - Max request body size

**Location Block:**
- `root <path>` - Root directory for this location
- `index <files>` - Default index files
- `allowedMethods <methods>` - Allowed HTTP methods (GET, POST, DELETE)
- `autoindex <on|off>` - Directory listing
- `uploadEnabled <on|off>` - Enable file uploads
- `uploadStore <path>` - Upload directory
- `cgiExtension <ext>` - CGI file extension (.py, .sh, etc.)
- `cgiPath <path>` - Path to CGI interpreter

### Common Body Size Limits
- 100 bytes - Stress testing
- 1 MB = 1048576 bytes - Normal usage
- 2 MB = 2097152 bytes - Medium files
- 5 MB = 5242880 bytes - CGI uploads
- 10 MB = 10485760 bytes - Large files

### Testing Methods

**GET:** Retrieve resources
```bash
curl http://localhost:8080/
```

**POST:** Upload data/files
```bash
curl -X POST -F "file=@test.txt" http://localhost:8080/upload
```

**DELETE:** Remove resources
```bash
curl -X DELETE http://localhost:8080/uploads/test.txt
```

**CGI with query string:**
```bash
curl "http://localhost:8080/python-cgi/test.py?name=value"
```

**CGI with POST data:**
```bash
curl -X POST -d "key=value" http://localhost:8080/python-cgi/test.py
```

---

## 42 Project Requirements Checklist

All configurations support testing these requirements:

- ✓ HTTP/1.1 compliance
- ✓ Multiple ports and servers
- ✓ Virtual hosting (server_name)
- ✓ GET, POST, DELETE methods
- ✓ Static file serving
- ✓ File upload
- ✓ Default error pages
- ✓ Client body size limits
- ✓ CGI execution (Python, Bash)
- ✓ Directory listing (autoindex)
- ✓ Default index files
- ✓ Non-blocking I/O (poll/epoll/kqueue)

---

## Troubleshooting

**Port already in use:**
```bash
lsof -i :8080
kill -9 <PID>
```

**Permission denied errors:**
- Check file permissions on www/ and cgi_examples/
- Ensure CGI scripts are executable (chmod +x)

**CGI not working:**
- Verify interpreter path (which python3, which bash)
- Check CGI script has execute permissions
- Verify cgiPath in config matches system path

**Upload fails:**
- Check uploadStore directory exists
- Verify write permissions on upload directory
- Check clientMaxBodySize is large enough

---

**Created for 42 webserv project - October 2024**
