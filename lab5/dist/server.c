#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

int main() {
    int sockfd;
    struct sockaddr_in server;
    struct sockaddr_in client;
    socklen_t len;
    int sock_client;
    char message[4194304];
    //char buf[65536];
    char hello[6] = "Hello\n";
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(12345);
    server.sin_addr.s_addr = INADDR_ANY;
    bind(sockfd, (struct sockaddr* )&server, sizeof(server));
    
    while (1) {
        pid_t pid = fork();
        if (pid < 0) {
            printf("fork error\n");
        }
        else if (pid == 0) {
            listen(sockfd, 100);
            len = sizeof(client);
            sock_client = accept(sockfd, (struct sockaddr*)&client, &len);
            send(sock_client, hello, sizeof(hello), 0);
            for (int i = 0; i < 10; ++i)
                send(sock_client, message, sizeof(message), 0);
            _exit(0);
        }
        else {
            wait(NULL);
        }
    }
    close(sock_client);
    close(sockfd);
    return 0;
}