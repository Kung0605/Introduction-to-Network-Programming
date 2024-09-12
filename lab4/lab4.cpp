#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string>
#include <sstream>
int main(){
    int socket_desc;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[65536];
    std::string name = "110652021";
    std::string url = "172.21.0.4";
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_desc < 0)
        std::cout<<"failed to create socket"<<std::endl;
    server = gethostbyname(url.c_str());
    if (server == NULL) 
        std::cout<<"could Not resolve hostname :("<<std::endl;
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(10001);
    bcopy((char *)server->h_addr,
            (char *)&serv_addr.sin_addr.s_addr,
            server->h_length);
    
    if(connect(socket_desc, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        std::cout<<"connection failed :("<<std::endl;
    std::string request = "GET /otp?name=" + name + " HTTP/1.1\r\nHost: 172.21.0.4:10001" + "\r\nConnection: keep-alive\r\n\r\n";
    if(send(socket_desc, request.c_str(), strlen(request.c_str())+1, 0) < 0){
        std::cout<<"failed to send request..."<<std::endl;
    }
    int n;
    std::string raw_site;
    while((n = recv(socket_desc, buffer, sizeof(buffer)+1, 0)) > 0){
        int i = 0;
        while (buffer[i] >= 32 || buffer[i] == '\n' || buffer[i] == '\r')
            raw_site+=buffer[i++];
    }
    unsigned int start = raw_site.find(name);
    unsigned int end = raw_site.find("==");
    std::string otp = raw_site.substr(start, end - start + 2);
    std::cout<<raw_site<<std::endl;
    close(socket_desc);
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if(connect(socket_desc, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        perror("connection");
    // Create an HTTP POST request
    std::stringstream upload_request;
    upload_request << "POST /upload HTTP/1.1 Host: 172.21.0.4:10001\r\n";
    upload_request << "Connection: keep-alive\r\n";
    upload_request << "Content-Length: 295\r\n";
    upload_request << "Cache-Control: max-age=0\r\n";
    upload_request << "Upgrade-Insecure-Requests: 1\r\n";
    upload_request << "Origin: 172.21.0.4:10001\r\n";
    upload_request << "Content-Type: multipart/form-data; boundary=----WebKitFormBoundarynTjVuzdx7vYKZrOa\r\n";
    upload_request << "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7\r\n";
    upload_request << "Referer: 172.21.0.4:10001/upload\r\n";
    upload_request << "Accept-Encoding: gzip, deflate\r\n";
    upload_request << "Accept-Language: en-US,en;q=0.9,zh-TW;q=0.8,zh;q=0.7\r\n\r\n\r\n";
    upload_request << "------WebKitFormBoundarynTjVuzdx7vYKZrOa\r\n";
    upload_request << "Content-Disposition: form-data; name=\"file\"; filename=\"a.txt\"\r\n";
    upload_request << "Content-Type: text/plain\r\n\r\n";
    upload_request << otp + "\n";
    upload_request << "------WebKitFormBoundarynTjVuzdx7vYKZrOa--\r\n";
    request = upload_request.str();
    upload_request.clear();
    std::cout << "-------------------\n" << request << "-------------------\n";
    if(send(socket_desc, request.c_str(), strlen(request.c_str()), 0) < 0){
        std::cout<<"failed to send request..."<<std::endl;
    }
    bzero(buffer, sizeof(buffer));
    raw_site = "";
    while((n = recv(socket_desc, buffer, sizeof(buffer)+1, 0)) > 0){
        int i = 0;
        while (buffer[i] >= 32 || buffer[i] == '\n' || buffer[i] == '\r')
            raw_site+=buffer[i++];
    }
    std::cout<<raw_site<<std::endl;
    return 0;
}