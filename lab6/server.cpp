/*
 * Lab problem set for INP course
 * by Chun-Ying Huang <chuang@cs.nctu.edu.tw>
 * License: GPLv2
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#define err_quit(m) { perror(m); exit(-1); }
long long int ack[1001];
char target_dir[64];
int fin[1001];
void solve_file(char* buf, int rlen) {
	int file_num, seq_num;
	sscanf(buf, "/%d SEQ:%d###", &file_num, &seq_num);
	if (seq_num == -1) {
		fin[file_num] = 1;
		return;
	}
	//printf("file_num:%05d SEQ:%05d\n", file_num, seq_num);
	//printf("buf:%s\n", buf);
	if (ack[file_num] != seq_num) {
		//printf("Wrong!\n");
		return;
	}
	ack[file_num]++;
	char file_name[128];
	bzero(file_name, sizeof(file_name));
	snprintf(file_name, 128,"%s/%06d", target_dir, file_num);
	int fd = open(file_name, O_APPEND | O_CREAT | O_RDWR, 0777);
	if (fd < 0) {
		perror("open: ");
		return;
	}

	char* data = buf + 20;
	write(fd, data, rlen - 20);
	close(fd);
}

int main(int argc, char *argv[]) {
	int s;
	struct sockaddr_in sin;
	if (argc < 4) {
		perror("usage: ./server <path-to-store-files> <total-number-of-files> <port>");
		return -1;
	}

	char *path_to_store_files = argv[1];
	int total_number_of_files = atoi(argv[2]);
	int port = atoi(argv[3]);

	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);



	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(strtol(argv[argc-1], NULL, 0));

	if((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		err_quit("socket");

	if(bind(s, (struct sockaddr*) &sin, sizeof(sin)) < 0)
		err_quit("bind");

	//change directory to files directory
	getcwd(target_dir, sizeof(target_dir));
	strcat(target_dir, path_to_store_files);
	if (chdir(target_dir) < -0) {
		perror("chdir: ");
	}

	for (int i = 0; i <= 1000; ++i) {
		ack[i] = 0;
	}
	while(1) {
		struct sockaddr_in csin;
		socklen_t csinlen = sizeof(csin);
		char buf[2048];
		int rlen;
		bzero(buf, sizeof(buf));
		if((rlen = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr*) &csin, &csinlen)) < 0) {
			perror("recvfrom");
			break;
		}
		solve_file(buf, rlen);
		//printf("%s\n", buf);
		sendto(s, buf, rlen, 0, (struct sockaddr*) &csin, sizeof(csin));
	}
	close(s);
}