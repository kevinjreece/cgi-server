//
//  ServerSocket.h
//  BasicWebServer
//
//  Created by Kevin J Reece on 1/28/15.
//  Copyright (c) 2015 Kevin J Reece. All rights reserved.
//

#ifndef BasicWebServer_ServerSocket_h
#define BasicWebServer_ServerSocket_h

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>

#define MAX_MSG_SZ 1024
// #define TEXT        "text/plain"
// #define HTML        "text/html"
// #define JPG         "image/jpg"
// #define GIF         "image/gif"
// #define DIRECTORY   "DIRECTORY"
// #define UNKNOWN     "UNKOWN"
// #define ERR         "ERR"
// #define CGI         "text/cgi"

using std::string;
using std::cout;
using std::vector;

enum FileType { CGI, TEXT, HTML, JPG, GIF, DIRECTORY, ERR, UNKNOWN };

class ServerSocket {
private:
    int _socket;
    string _path;
    FileType _type;
    string _content;
    string _status;
    string _directory;
    string _home_dir;
    std::map<FileType, string> _type_map;

    void initMap() {
        _type_map[TEXT] = "text/plain";
        _type_map[HTML] = "text/html";
        _type_map[JPG] = "image/jpg";
        _type_map[GIF] = "image/gif";
    }
    
    // Determine if the character is whitespace
    bool isWhitespace(char c) {
        
        switch (c) {
            case '\r':
            case '\n':
            case ' ':
            case '\0':
                return true;
            default:
                return false;
        }
    }
    
    // Strip off whitespace characters from the end of the line
    void chomp(char *line) {
        int len = (int) strlen(line);
        
        while (isWhitespace(line[len])){
            line[len--] = '\0';
        }
    }
    
    // Read the line one character at a time, looking for the CR
    char * GetLine(int fds) {
        char tline[MAX_MSG_SZ];
        char *line;
        char* err = new char[1];
        
        int messagesize = 0;
        int amtread = 0;
        
        while((amtread = (int) read(fds, tline + messagesize, 1)) < MAX_MSG_SZ) {
            if (amtread > 0) {
                messagesize += amtread;
            }
            else {
                perror("Socket Error is:");
                fprintf(stderr, "Read Failed on file descriptor %d messagesize = %d\n", fds, messagesize);
                _type = ERR;
                return err;
            }
            
            //fprintf(stderr,"%d[%c]", messagesize,message[messagesize-1]);
            if (tline[messagesize - 1] == '\n') {
                break;
            }
        }
        
        tline[messagesize] = '\0';
        chomp(tline);
        line = (char *)malloc((strlen(tline) + 1) * sizeof(char));
        strcpy(line, tline);
        //fprintf(stderr, "GetLine: [%s]\n", line);
        return line;
    }
    
    // Change to upper case and replace with underlines for CGI scripts
    void UpcaseAndReplaceDashWithUnderline(char *str) {
        int i;
        char *s;
        
        s = str;
        for (i = 0; s[i] != ':'; i++) {
            if (s[i] >= 'a' && s[i] <= 'z') {
                s[i] = 'A' + (s[i] - 'a');
            }
            
            if (s[i] == '-') {
                s[i] = '_';
            }
        }
        
    }
    
    // When calling CGI scripts, you will have to convert header strings
    // before inserting them into the environment.  This routine does most
    // of the conversion
    char *FormatHeader(char *str, char *prefix) {
        char *result = (char *)malloc(strlen(str) + strlen(prefix));
        char* value = strchr(str,':') + 2;
        UpcaseAndReplaceDashWithUnderline(str);
        *(strchr(str,':')) = '\0';
        sprintf(result, "%s%s=%s", prefix, str, value);
        return result;
    }
    
    // Get the header lines from a socket
    //   envformat = true when getting a request from a web client
    //   envformat = false when getting lines from a CGI program
    void GetHeaderLines(vector<char *> &headerLines, int skt, bool envformat) {
        // Read the headers, look for specific ones that may change our responseCode
        char *line;
        char *tline;
        
        tline = GetLine(skt);
        while(strlen(tline) != 0) {
            if (strstr(tline, "Content-Length") ||
                strstr(tline, "Content-Type")) {
                if (envformat) {
                    char empty_str[] = "";
                    line = FormatHeader(tline, empty_str);
                }
                else {
                    line = strdup(tline);
                }
            }
            else {
                if (envformat) {
                    char http[] = "HTTP_";
                    line = FormatHeader(tline, http);
                }
                else {
                    line = (char *)malloc((strlen(tline) + 10) * sizeof(char));
                    sprintf(line, "HTTP_%s", tline);
                }
            }
            //fprintf(stderr, "Header --> [%s]\n", line);
            
            headerLines.push_back(line);
            free(tline);
            tline = GetLine(skt);
        }
        free(tline);
    }
    
public:
    ServerSocket(int socket, string home_dir) {
        _socket = socket;
        _home_dir = home_dir;
        initMap();
    }
    
    void sendStandardResponse() {
        string message = _status +
                            "Content-Type: " + _type_map[_type] + "\r\n\r\n" +
                            _content + "\r\n";
        write(_socket, message.c_str(), message.length());
        return;
    }
    
    string getFileName(string start_line) {
        return start_line.substr(4, start_line.length() - 13);
    }
    
    string getExtension(string file_name) {
        string ext = "";
        int periods_pos = (int)file_name.find_last_of(".");
        
        if (periods_pos < 0) {
            ext = "";
        }
        else {
            ext = file_name.substr(periods_pos + 1);
        }
        
        return ext;
    }
    
    void setContentType(string ext) {
        
        if (ext == "txt") {
            _type = TEXT;
        }
        else if (ext == "html") {
            _type = HTML;
        }
        else if (ext == "jpg") {
            _type = JPG;
        }
        else if (ext == "gif") {
            _type = GIF;
        }
        else if (ext == "cgi" || ext == "pl") {
            _type = CGI;
        }
        else if (ext == "") {
            _type = DIRECTORY;
        }
        else {
            _type = UNKNOWN;
        }
        
        return;
    }
    
    void parseFirstLine(string start_line) {
        
        if(!strstr(start_line.c_str(), "GET")) {
            _type = ERR;
            return;
        }
        
        // Find path
        _path = _home_dir + getFileName(start_line);
        cout << "File: \"" + _path + "\"\n";
        
        // Find extension type
        string ext = getExtension(_path);
        cout << "Extension: \"" + ext + "\"\n";
        
        // Find and set content type
        setContentType(ext);
        cout << "Content Type: \"" + _type_map[_type] + "\"\n";
        return;
    }
    
    void processDirectory() {
        DIR *dirp;
        struct dirent *dp;
        std::stringstream listing;
        
        listing << "<html>\n<body>\n<ul>\n";
        
        dirp = opendir(_directory.c_str());
        while ((dp = readdir(dirp)) != NULL) {
            listing << "<li><a href=\"" << dp->d_name << "\">" << dp->d_name << "</a></li>\n";
        }
        (void)closedir(dirp);
        
        listing << "</ul>\n</body>\n</html>\n";
        _content = listing.str();
        return;
    }
    
    void processPath() {
        if (_type == DIRECTORY) {// Is the path to a directory?
            _type = HTML;
            _directory = _path;
            _path += "/index.html";
            _status = "HTTP/1.0 200 OK\r\n";
            std::ifstream index_ifs(_path.c_str());
            if (!index_ifs) {// Does the directory have an index.html file?
                processDirectory();
                return;
            }
        }
        
        std::ifstream ifs(_path.c_str());
        if (ifs) {
            _status = "HTTP/1.0 200 OK\r\n";
            string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
            _content = str;
        }
        else {
            _status = "HTTP/1.0 404 Not Found\r\n";
            _type = HTML;
            std::ifstream not_found_ifs("notFound.html");
            string str((std::istreambuf_iterator<char>(not_found_ifs)), std::istreambuf_iterator<char>());
            _content = str;
        }
    }
    
    void processRequest() {
        vector<char *> header_lines;
        char content_type[MAX_MSG_SZ];
        int content_length = 0;
        
        // First read the status line
        string start_line = GetLine(_socket);
        
        if (_type == ERR) {
            return;
        }
        
        cout << "Status line: " + start_line + "\n";
        
        parseFirstLine(start_line);
        
        if (_type == ERR) {
            return;
        }
        else if (_type == CGI) {
            runCGI();
            return;
        }
        
        processPath();
        
        // Read the header lines
        GetHeaderLines(header_lines, _socket, false);
        
        // Now print them out
        cout << "\nRequest Headers:\n";
        
        for (int i = 0; i < header_lines.size(); i++) {
            printf("[%d] %s\n",i,header_lines[i]);
            
            if(strstr(header_lines[i], "Content-Type")) {
                sscanf(header_lines[i], "Content-Type: %s", content_type);
            }
            else if(strstr(header_lines[i], "Content-Length")) {
                sscanf(header_lines[i], "Content-Length: %d", &content_length);
            }
        }
        
        sendStandardResponse();
    }
    
    bool closeSocket() {
        if (shutdown(_socket, SHUT_RDWR) && close(_socket) == -1) {
            return false;
        }
        else {
            return true;
        }
    }

    void runCGI() {

        pid_t pID = fork();
        if (pID == 0) {
            char* argv_to_child[2];
            char* env_to_child[2];
            argv_to_child[0] = (char *)"foo";
            argv_to_child[1] = NULL;
            env_to_child[0] = (char *)"bar";
            env_to_child[1] = NULL;
            
            execve(_path.c_str(), argv_to_child, env_to_child);
        }
        else {
            int stat;
            int err;
            err = wait(&stat);
        }

    }
    
};

#endif
