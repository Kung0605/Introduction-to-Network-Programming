/*
 *  Lab problem set for INP course
 *  by Chun-Ying Huang <chuang@cs.nctu.edu.tw>
 *  License: GPLv2
 */
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <iostream>
#include <sys/select.h>
#include <string>
using namespace std;
#define NIPQUAD(m)	((unsigned char*) &(m))[0], ((unsigned char*) &(m))[1], ((unsigned char*) &(m))[2], ((unsigned char*) &(m))[3]
#define errquit(m)	{ perror(m); exit(-1); }

#define MYADDR		0x0a0000fe
#define ADDRBASE	0x0a00000a
#define	NETMASK		0xffffff00
#define MAX_BUF 1400

int
tun_alloc(char *dev) {
	struct ifreq ifr;
	int fd, err;
	if((fd = open("/dev/net/tun", O_RDWR)) < 0 )
		return -1;
	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = IFF_TUN | IFF_NO_PI;	/* IFF_TUN (L3), IFF_TAP (L2), IFF_NO_PI (w/ header) */
	if(dev && dev[0] != '\0') strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	if((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ) {
		close(fd);
		return err;
	}
	if(dev) strcpy(dev, ifr.ifr_name);
	return fd;
}

int
ifreq_set_mtu(int fd, const char *dev, int mtu) {
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_mtu = mtu;
	if(dev) strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	return ioctl(fd, SIOCSIFMTU, &ifr);
}

int
ifreq_get_flag(int fd, const char *dev, short *flag) {
	int err;
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	if(dev) strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	err = ioctl(fd, SIOCGIFFLAGS, &ifr);
	if(err == 0) {
		*flag = ifr.ifr_flags;
	}
	return err;
}

int
ifreq_set_flag(int fd, const char *dev, short flag) {
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	if(dev) strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	ifr.ifr_flags = flag;
	return ioctl(fd, SIOCSIFFLAGS, &ifr);
}

int
ifreq_set_sockaddr(int fd, const char *dev, int cmd, unsigned int addr) {
	struct ifreq ifr;
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = addr;
	memset(&ifr, 0, sizeof(ifr));
	memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));
	if(dev) strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	return ioctl(fd, cmd, &ifr);
}

int
ifreq_set_addr(int fd, const char *dev, unsigned int addr) {
	return ifreq_set_sockaddr(fd, dev, SIOCSIFADDR, addr);
}

int
ifreq_set_netmask(int fd, const char *dev, unsigned int addr) {
	return ifreq_set_sockaddr(fd, dev, SIOCSIFNETMASK, addr);
}

int
ifreq_set_broadcast(int fd, const char *dev, unsigned int addr) {
	return ifreq_set_sockaddr(fd, dev, SIOCSIFBRDADDR, addr);
}
void configure_tun0_server(int fd) {
    ifreq_set_mtu(fd, "tun0", 1400);
    ifreq_set_addr(fd, "tun0", htonl(0x0a0000fe));  // Assuming server address is 10.0.0.254
    ifreq_set_broadcast(fd, "tun0", htonl(0x0a0000ff));
    ifreq_set_netmask(fd, "tun0", htonl(0xffffff00));
    short flags;
    ifreq_get_flag(fd, "tun0", &flags);
	ifreq_set_flag(fd, "tun0", IFF_UP|flags);
	
}
void configure_tun0_client(int fd,int number) {
    ifreq_set_mtu(fd, "tun0", 1400);
    ifreq_set_addr(fd, "tun0", htonl(0x0a000001+number));
    ifreq_set_broadcast(fd, "tun0", htonl(0x0a0000ff));
    ifreq_set_netmask(fd, "tun0", htonl(0xffffff00));
    short flags;
    ifreq_get_flag(fd, "tun0", &flags);
	ifreq_set_flag(fd, "tun0", IFF_UP|flags);
	
}
char* get_ip(const char* server_name) {
    struct addrinfo hints, *result, *p;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP
    int status = getaddrinfo(server_name, nullptr, &hints, &result);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return nullptr;
    }

    char* ipaddr = nullptr;
    for (p = result; p != nullptr; p = p->ai_next) {
        struct sockaddr_in* addr = (struct sockaddr_in*)p->ai_addr;
        ipaddr = (char*)malloc(INET_ADDRSTRLEN);
        if (ipaddr == nullptr) {
            perror("malloc");
            freeaddrinfo(result);
            return nullptr;
        }
        inet_ntop(AF_INET, &(addr->sin_addr), ipaddr, INET_ADDRSTRLEN);
        break;
    }
    freeaddrinfo(result);
    return ipaddr;
}
int tunvpn_server(int port) {
	int number=1;
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    char name[]="tun0";
	int tun_fd = tun_alloc(name);
    printf("## [server] tun0 fd = %d\n", tun_fd);
	configure_tun0_server(server_fd);
    
    printf("## [server] starts ...\n");

    sockaddr_in server_addr,client1_addr,client2_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;


    // Bind the socket
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 5) == -1) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    char buffer[MAX_BUF];
    fd_set read_set;
    socklen_t addr_len = sizeof(client1_addr);
    printf("## [server] waiting for client1 connection ...\n");
    int client1_fd = accept(server_fd, (struct sockaddr*)&client1_addr, &addr_len);
    if (client1_fd == -1) {
        perror("accept");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    cout << "## [server] Client1 connected." << endl;
    write(client1_fd, &number, sizeof(number));
    number++;
    addr_len = sizeof(client2_addr);
    printf("## [server] waiting for client2 connection ...\n");
    int client2_fd = accept(server_fd, (struct sockaddr*)&client2_addr, &addr_len);
    if (client2_fd == -1) {
        perror("accept");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    cout << "## [server] Client2 connected." << endl;
    write(client2_fd, &number, sizeof(number));
    cout<<"buffer="<<buffer<<endl;
    snprintf(buffer, sizeof(buffer), "%d", number);
    cout<<"buffer="<<buffer<<endl;
    while (1) {
        
        FD_ZERO(&read_set);
        FD_SET(tun_fd, &read_set);
        FD_SET(client1_fd, &read_set);
        FD_SET(client2_fd, &read_set);

        //int max_fd = max(tun_fd, client1_fd) + 1;
        int max_fd2 = max(tun_fd, client2_fd) + 1;

        if (select(max_fd2, &read_set, nullptr, nullptr, nullptr) == -1) {
            perror("select");
            close(server_fd);
            close(client1_fd);
            exit(EXIT_FAILURE);
        }

            if (FD_ISSET(tun_fd, &read_set)) {
                int bytes_read = read(tun_fd, buffer, MAX_BUF);
                // printf("server read %zd bytes from s: %s\n", bytes_read, buffer);
				struct iphdr *ip_header = (struct iphdr *)buffer;
				//printf("Version: %u\n", ip_header->version);
				//printf("Header length: %u\n", ip_header->ihl);
				//printf("Total length: %u\n", ntohs(ip_header->tot_len));
				//printf("Source IP: %s\n", inet_ntoa(*(struct in_addr *)&(ip_header->saddr)));
				//printf("Destination IP: %s\n", inet_ntoa(*(struct in_addr *)&(ip_header->daddr)));
                if (bytes_read > 0) {
                    
                    if(strcmp(inet_ntoa(*(struct in_addr *)&(ip_header->daddr)),"10.0.0.2")==0){
                        write(client1_fd, buffer, bytes_read);
                       // cout<<"write to client1_fd"<<endl;
                    }else if(strcmp(inet_ntoa(*(struct in_addr *)&(ip_header->daddr)),"10.0.0.3")==0){
                        write(client2_fd, buffer, bytes_read);
                        //cout<<"write to client2_fd"<<endl;
                    }
                }
            }

            if (FD_ISSET(client2_fd, &read_set)) {
                //cout<<"here2"<<endl;
                int bytes_read = read(client2_fd, buffer, MAX_BUF);
                if (bytes_read > 0) {
                    //cout << "Received from client2_fd: " << buffer << endl;
                    write(tun_fd, buffer, bytes_read);
                }
            }
            if (FD_ISSET(client1_fd, &read_set)) {
                //cout<<"here1"<<endl;
                int bytes_read = read(client1_fd, buffer, MAX_BUF);
                if (bytes_read > 0) {
                    //cout << "Received from client1_fd: " << buffer << endl;
                    write(tun_fd, buffer, bytes_read);
                }
            }
            
    }
    //close(client1_fd);
    close(server_fd);
    
    return 0;
}


int tunvpn_client(const char *server, int port) {
    // Create a socket for client
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
	
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(get_ip(server));
    
	int c=connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (c < 0) {
		perror("connect");
		cerr << "Connection failed. Error details: " << strerror(errno) << endl;
		close(client_fd);
		return -1; 
	}
    char name[]="tun0";
    int tun_fd = tun_alloc(name);
    int number;
    read(client_fd, &number, sizeof(number));
    //printf("number=%d\n",number);
    configure_tun0_client(client_fd,number);
    fd_set read_set;
    char buffer[MAX_BUF];
    char numberStr[10]; 
    snprintf(numberStr, sizeof(numberStr), "%d", number);
    // cout<<"numberStr="<<numberStr<<endl;
    while (1) {

        FD_ZERO(&read_set);
        FD_SET(tun_fd, &read_set);
        FD_SET(client_fd, &read_set);

        int max_fd = max(tun_fd, client_fd) + 1;

        if (select(max_fd, &read_set, nullptr, nullptr, nullptr) == -1) {
            perror("select");
            close(client_fd);
            exit(EXIT_FAILURE);
        }else{
            if (FD_ISSET(tun_fd, &read_set)) {
                int bytes_read = read(tun_fd, buffer, MAX_BUF);
                if (bytes_read > 0) {
                    struct iphdr *ip_header = (struct iphdr *)buffer;
                    //printf("Version: %u\n", ip_header->version);
                    //printf("Header length: %u\n", ip_header->ihl);
                    //printf("Total length: %u\n", ntohs(ip_header->tot_len));
                    //printf("Source IP: %s\n", inet_ntoa(*(struct in_addr *)&(ip_header->saddr)));
                    //printf("Destination IP: %s\n", inet_ntoa(*(struct in_addr *)&(ip_header->daddr)));
                    //cout << "Received from tun_fd: " << buffer << endl;
                    if(strcmp(inet_ntoa(*(struct in_addr *)&(ip_header->saddr)),"10.0.0.2")==0){
                        if(number==1){
                            write(client_fd, buffer, bytes_read);
                            //cout<<"cliend1 write to server"<<endl;
                        }else{
                           // cout<<"wrong number: "<<number<<endl;
                        }
                    }else if(strcmp(inet_ntoa(*(struct in_addr *)&(ip_header->saddr)),"10.0.0.3")==0){
                        if(number==2){
                            write(client_fd, buffer, bytes_read);
                            //cout<<"cliend2 write to server"<<endl;
                        }else{
                            //cout<<"wrong number: "<<number<<endl;
                        }
                    }else{

                        //cout<<"wrong destination"<<endl;
                    }
                    
                }
            }

            if (FD_ISSET(client_fd, &read_set)) {
                //cout<<"client_fd="<<client_fd<<endl;
                int bytes_read = read(client_fd, buffer, MAX_BUF);
                if (bytes_read > 0) {
                    //cout << "Received from client_fd: "<<number<<" "<< buffer << endl;
                    if(write(tun_fd, buffer, bytes_read)==0){
                        //cout<<"write to tun_fd failed"<<endl;
                    }
                }
            }
        }

       
    }

    return 0;
}

int
usage(const char *progname) {
	fprintf(stderr, "usage: %s {server|client} {options ...}\n"
		"# server mode:\n"
		"	%s server port\n"
		"# client mode:\n"
		"	%s client servername serverport\n",
		progname, progname, progname);
	return -1;
}
int main(int argc, char *argv[]) {
    if (argc < 3) {
        return usage(argv[0]);
    }

    if (strcmp(argv[1], "server") == 0) {
        if (argc < 3) {
            return usage(argv[0]);
        }
        int port = strtol(argv[2], nullptr, 0);
        if (port <= 0 || port > 65535) {
            cerr << "Invalid port number. Port must be between 1 and 65535." << endl;
            return -1;
        }
        return tunvpn_server(port);
    } else if (strcmp(argv[1], "client") == 0) {
        if (argc < 4) {
            return usage(argv[0]);
        }

        const char* serverAddress = argv[2];
        int port = strtol(argv[3], nullptr, 0);
        if (port <= 0 || port > 65535) {
            cerr << "Invalid port number. Port must be between 1 and 65535." << endl;
            return -1;
        }
        return tunvpn_client(serverAddress, port);
    } else {
        fprintf(stderr, "## unknown mode %s\n", argv[1]);
        return -1;
    }

    return 0;
}