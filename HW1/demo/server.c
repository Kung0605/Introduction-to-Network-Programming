#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <pthread.h>

#define errquit(m)	{ perror(m); exit(-1); }
#define SEND_BUF_SIZE 524288
#define RECV_BUF_SIZE 32768
#define HEADER_LEN 2048
#define CONTENT_LEN 7000000
#define NUM_THREADS 1

typedef struct {
    int fd;
    off_t start;
    off_t end;
    char *content;
} ThreadData;
enum Content_Type {
    text_html,
    text_plain,
    image_jpeg,
    image_png,
    audio_mp3,
    forbidden
};
//FILE *output;
static int port_http = 80;
static int port_https = 443;
static const char *docroot = "/html";
enum Content_Type get_content_type(char path[]) {
    char *ext = strrchr(path, '.');
    if (ext == NULL) return text_html;
    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) return text_html;
    if (strcmp(ext, ".txt") == 0) return text_plain;
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return image_jpeg;
    if (strcmp(ext, ".png") == 0) return image_png;
    if (strcmp(ext, ".mp3") == 0) return audio_mp3;
    return forbidden;
}
int is_regular_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}
char* decode_path(char path[]) {
    static char decoded_path[256];
    char *decoded_path_ptr = decoded_path;
    char *path_ptr = path;
    while (*path_ptr) {
        if (*path_ptr == '%' && path_ptr[1] && path_ptr[2]) {
            char hex[3] = {path_ptr[1], path_ptr[2], '\0'};
            *decoded_path_ptr++ = (char)strtol(hex, NULL, 16);
            path_ptr += 3;
        } else {
            *decoded_path_ptr++ = *path_ptr++;
        }
    }
    *decoded_path_ptr = '\0';
    return decoded_path;
}

int open_file(char path[], int* status_code, char content[]) {
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        *status_code = 404;
        sprintf(content, "404 Not Found");
    }
    return fd;
}

void *read_file_part(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    lseek(data->fd, data->start, SEEK_SET);
    read(data->fd, data->content, data->end - data->start);
    return NULL;
}

int read_file(int fd, char content[], off_t file_size) {
    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];
    off_t part_size = file_size / NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].fd = fd;
        thread_data[i].start = i * part_size;
        thread_data[i].end = (i == NUM_THREADS - 1) ? file_size : (i + 1) * part_size;
        thread_data[i].content = content + thread_data[i].start;
        pthread_create(&threads[i], NULL, read_file_part, &thread_data[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    close(fd);
    return file_size;
}

int set_content(char path[], char content[], enum Content_Type content_type, int* status_code, char method[]) {
    char* decoded_path = decode_path(path);
    // Ignore query string
    char *query_string = strchr(decoded_path, '?');
    if (query_string) *query_string = '\0';

    if (strcmp(decoded_path, "/") == 0) 
        sprintf(decoded_path, "/index.html");

    char complete_path[64];
    strcpy(complete_path, docroot);
    strcat(complete_path, decoded_path);

    int fd = open_file(complete_path, status_code, content);
    if (fd == -1) return strlen(content);

    if (strcmp(method, "GET") != 0) {
        *status_code = 501;
        sprintf(content, "501 Not Implemented");
        return strlen(content);
    }

    char *slash = strrchr(complete_path, '/');
    if (slash && !strcmp(slash, "/")) {
        *status_code = 403;
        sprintf(content, "403 Forbidden");
        return strlen(content);
    }

    if (!is_regular_file(complete_path)) {
        *status_code = 301;
        sprintf(content, "301 Moved Permanently");
        return strlen(content);
    }

    *status_code = 200;
    off_t file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    memset(content, 0, CONTENT_LEN);

    return read_file(fd, content, file_size);
}
int set_header(char header[], int content_len, enum Content_Type content_type, int status_code, char path[]) {
    const char *type_str;
    switch (content_type) {
        case text_html:
            type_str = "Content-Type: text/html; charset=UTF-8\r\n";
            break;
        case text_plain:
            type_str = "Content-Type: text/plain; charset=UTF-8\r\n";
            break;
        case image_jpeg:
            type_str = "Content-Type: image/jpeg;\r\n";
            break;
        case image_png:
            type_str = "Content-Type: image/png;\r\n";
            break;
        case audio_mp3:
            type_str = "Content-Type: audio/mpeg;\r\n";
            break;
        default:
            type_str = "";
            break;
    }
    int header_len = snprintf(header, 1024, "HTTP/1.1 %d %s\r\nLocation: %s%c\r\n%sContent-Length: %d\r\nConnection: close\r\n\r\n",
                              status_code, (status_code == 200 ? "OK" : ""), path, '/', type_str, content_len);
    return header_len;
}
int main(int argc, char *argv[]) {
	int s;
	struct sockaddr_in sin;
	char sending_buffer[SEND_BUF_SIZE];
	char receiving_buffer[RECV_BUF_SIZE];
	if(argc > 1) { port_http  = strtol(argv[1], NULL, 0); }
	if(argc > 2) { if((docroot = strdup(argv[2])) == NULL) errquit("strdup"); }
	if(argc > 3) { port_https = strtol(argv[3], NULL, 0); }
	if((s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) errquit("socket");
	do {
		int v = 1;
		setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &v, sizeof(v));
	} while(0);
	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(80);
	if(bind(s, (struct sockaddr*) &sin, sizeof(sin)) < 0) errquit("bind");
	while (1) {
        pid_t pid = fork();
        if (pid < 0) {
            printf("fork error\n");
        }
        else if (pid == 0) {
            listen(s, 100);
            memset(receiving_buffer, 0, sizeof(receiving_buffer));
			int c;
			struct sockaddr_in csin;
			socklen_t csinlen = sizeof(csin);
			if((c = accept(s, (struct sockaddr*) &csin, &csinlen)) < 0) {
				perror("accept");
				continue;
			}
			int n;
			if((n = recv(c, receiving_buffer, sizeof(receiving_buffer), 0)) < 0) {
				perror("recv");
				close(c);
				continue;
			}
			char path[64];
			char method[64];
			memset(path, 0, sizeof(path));
			sscanf(receiving_buffer, "%s %s HTTP/1.1", method, path);
			char content[CONTENT_LEN];
			char header[HEADER_LEN];
			enum Content_Type content_type = get_content_type(path);
			int status_code;
			int content_len = set_content(path, content, content_type,&status_code, method);
			int header_len = set_header(header, content_len, content_type, status_code, path);
			memcpy(sending_buffer, header, header_len);
			memcpy(sending_buffer + header_len, content, content_len);
			
			send(c, sending_buffer, header_len + content_len, 0);
			close(c);
            _exit(0);
        }
        else {
            wait(NULL);
        }
    }
	return 0;
}
