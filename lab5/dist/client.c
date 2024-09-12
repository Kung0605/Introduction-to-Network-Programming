#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <stdlib.h>

int compare(const void *a, const void *b) {
    int c = *(int *)a;
    int d = *(int *)b;
    return d > c;  
}
int main() {
    struct sockaddr_in server;
    int sockfd;
    char buf[1048576];
    char hello[6] = "Hello\n";
    long n;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(12345);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr.s_addr);
    connect(sockfd, (struct sockaddr*)&server, sizeof(server));
    memset(buf, 0, sizeof(buf));
    struct timeval start, end;
    send(sockfd, hello, sizeof(hello), 0);
    gettimeofday(&start, NULL);
    n = recv(sockfd, buf, sizeof(buf), 0);
    gettimeofday(&end, NULL);
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;
    long delay = (seconds * 1000 + microseconds / 1000) / 2;
    gettimeofday(&start, NULL);
    int bws[16384];
    int index = 0;
    while ((n = recv(sockfd, buf, sizeof(buf), 0)) > 0) {
        gettimeofday(&end, NULL);
        long seconds = end.tv_sec - start.tv_sec;
        long microseconds = end.tv_usec - start.tv_usec;
        double delay = (seconds * 1000000 + microseconds);
        double bandwidth = n * 8 / ((abs(delay) <= 1e-5 ? 1 : delay));
        bws[index++] = (int)bandwidth;
        gettimeofday(&start, NULL);
    }
    qsort(bws, index, sizeof(int), compare);
    long bandwidth = 0;
    for (int i = 325; i < 500; ++i) 
        bandwidth += bws[i];
    bandwidth /= 170;
    //printf("# RECEIVED: %ld bytes in %ld ms\n", bytes, totaltime);
    printf("# RESULTS: delay = %ld ms, bandwidth = %ld Mbps\n", delay + 1, bandwidth);
    close(sockfd);
    return 0;
}