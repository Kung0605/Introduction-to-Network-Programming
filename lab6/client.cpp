/*
 * Lab problem set for INP course
 * by Chun-Ying Huang <chuang@cs.nctu.edu.tw>
 * License: GPLv2
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#define CHUNK_SIZE 1024
#define SEND_TIME 20
#define err_quit(m) { perror(m); exit(-1); }

#define NIPQUAD(s)	((unsigned char *) &s)[0], \
					((unsigned char *) &s)[1], \
					((unsigned char *) &s)[2], \
					((unsigned char *) &s)[3]

static int s = -1;
static struct sockaddr_in sin;
static uint32_t seq = 0;
static unsigned count = 0;

void sendfile(char *file_name) {
	int fd = open(file_name, O_RDONLY);
	if (fd < 0)
		return;
	off_t file_size = lseek(fd, 0, SEEK_END);
	uint32_t seq = 0;
	lseek(fd, 0, SEEK_SET);
	char chunk[CHUNK_SIZE + 20];
	file_name = strrchr(file_name, '/');
	char file_name_chunk[20];
	snprintf(file_name_chunk, 20,"%s SEQ:%05d###", file_name, seq++);
	file_name_chunk[19] = '#';
	for (int i = 0; i < SEND_TIME; ++i)
		sendto(s, file_name_chunk, strlen(file_name_chunk), 0, (struct sockaddr*) &sin, sizeof(sin));
	while (1) {
		bzero(chunk, sizeof(chunk));
		char header[20];
		snprintf(header, 20,"%s SEQ:%05d###", file_name, seq++);
		//printf("%s\n", header);
		int read_size = read(fd, chunk + 20, CHUNK_SIZE);
		memcpy(chunk, header, sizeof(header));
		chunk[19] = '#';
		//printf("%s\n", chunk);
		if (read_size < 0) {
			perror("read: ");
			break;
		}
		if (read_size == 0)
			break;
		for (int i = 0; i < SEND_TIME; ++i) {
			sendto(s, chunk, read_size + 20, 0, (struct sockaddr*) &sin, sizeof(sin));
			usleep(0.8 * 1000);
		}
	}
}
int main(int argc, char *argv[]) {
	if (argc < 4) {
		perror("usage: /client <path-to-read-files> <total-number-of-files> <port> <server-ip-address>");
		return -1;
	}

	char *path_to_read_files = argv[1];
	int total_number_of_files = atoi(argv[2]);
	int port = atoi(argv[3]);
	char *server_ip_address = argv[4];

	srand(time(0) ^ getpid());

	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	if(inet_pton(AF_INET, server_ip_address, &sin.sin_addr) != 1) {
		return -fprintf(stderr, "** cannot convert IPv4 address for %s\n", argv[1]);
	}

	if((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		err_quit("socket");

	//signal(SIGALRM, do_send);

	seq = rand() % 0xffffff;
	char target_dir[64];
	getcwd(target_dir, sizeof(target_dir));
	strcat(target_dir, path_to_read_files);
	if (chdir(target_dir) < -0) {
		perror("chdir: ");
	}
	for (int i = 0; i <= total_number_of_files; ++i) {
		char file_num[20];
		sprintf(file_num, "/%06d", i);
		char file_name[128];
		bzero(file_name, sizeof(file_name));
		memcpy(file_name, target_dir, strlen(target_dir));
		strcat(file_name, file_num);
		//printf("%s\n", file_name);
		int fd = open(file_name, O_RDONLY);
		if (fd < 0) 
			continue;
		sendfile(file_name);
	}
	close(s);
}