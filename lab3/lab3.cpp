#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#include <iostream>
#include <vector>
#include <fstream>
#include<queue>
#include <algorithm>
#define SA struct sockaddr
#define MAX 65536
#define RIGHT 'd'
#define LEFT 'a'
#define UP 'w'
#define DOWN 's'
int port;
struct point {
	int x;
	int y;
};
std::string bfs(std::vector<std::vector<char>> maze) {
	int m = maze.size();
	int n = maze[0].size();
	std::vector<std::vector<char>> move(m, std::vector<char> (n, ' ')); 
	point start, end;
	for (int i = 0; i < m; ++i) {
		for (int j = 0; j < n; ++j) {
			if (maze[i][j] == '*') {
				start.x = i;
				start.y = j;
			}
			if (maze[i][j] == 'E') {
				end.x = i;
				end.y = j;
			}
		}
	}
	std::queue<point> queue;
	queue.push(start);
	while (!queue.empty()) {
		point tmp = queue.front();
		queue.pop();
		std::cout << tmp.x << " " << tmp.y << std::endl;
		if (tmp.x + 1 < m && maze[tmp.x + 1][tmp.y] == '.' || maze[tmp.x + 1][tmp.y] == 'E') {
			queue.push({tmp.x + 1, tmp.y});
			maze[tmp.x + 1][tmp.y] = '-';
			move[tmp.x + 1][tmp.y] = 's';
		}
		if (tmp.x - 1 >= 0 && maze[tmp.x - 1][tmp.y] == '.' || maze[tmp.x - 1][tmp.y] == 'E') {
			queue.push({tmp.x - 1, tmp.y});
			maze[tmp.x - 1][tmp.y] = '-';
			move[tmp.x - 1][tmp.y] = 'w';
		}
		if (tmp.y + 1 < n && maze[tmp.x][tmp.y + 1] == '.' || maze[tmp.x][tmp.y + 1] == 'E') {
			queue.push({tmp.x, tmp.y + 1});
			maze[tmp.x][tmp.y + 1] = '-';
			move[tmp.x][tmp.y + 1] = 'd';
		}
		if (tmp.y - 1 >= 0 && maze[tmp.x][tmp.y - 1] == '.' || maze[tmp.x][tmp.y - 1] == 'E') {
			queue.push({tmp.x, tmp.y - 1});
			maze[tmp.x][tmp.y - 1] = '-';
			move[tmp.x][tmp.y - 1] = 'a';
		}
	}
	std::string result;
	while (end.x != start.x || end.y != start.y) {
		result += move[end.x][end.y];
		if (move[end.x][end.y] == 's') 
			end.x--;
		else if (move[end.x][end.y] == 'w') 
			end.x++;
		else if (move[end.x][end.y] == 'd') 
			end.y--;
		else if (move[end.x][end.y] == 'a') 
			end.y++;
	}
	reverse(result.begin(), result.end());
	return result;
}
std::vector<std::vector<char>> create_maze_1(char buffer[], int n) {
    std::vector<std::vector<char>> maze(7, std::vector<char>(11, ' '));
    std::string str(buffer);
    //std::cout<<"str "<<str<<"end"<<std::endl;
    int i = 0, j = 0;
    for (char c : str) {
        if (c != '#' && c != '.' && c != '*' && c != 'E')
            continue;
        if (j == 11) {
            j = 0;
            i++;
        }
        if (i == 7)
            break;
        //std::cout << "i : " << i << " j : " << j <<std::endl;
        maze[i][j++] = c;
    }
    return maze;
}
std::vector<std::vector<char>> create_maze_2(char buffer[], int n) {
    std::vector<std::vector<char>> maze(21, std::vector<char>(79, ' '));
    std::string str(buffer);
    std::cout<<"str"<<str<<std::endl;
    int i = 0, j = 0;
    for (char c : str) {
        if (c != '#' && c != '.' && c != '*' && c != 'E')
            continue;
        if (j == 79) {
            j = 0;
            i++;
        }
        if (i == 21)
            break;
        //std::cout << "i : " << i << " j : " << j <<std::endl;
        maze[i][j++] = c;
    }
    return maze;
}

std::vector<std::vector<char>> create_maze_3(int sockfd,char buffer[], int n) {
    std::vector<std::vector<char>> maze(101, std::vector<char>(101, ' '));
    int i = 0, j = 0;
    int l_r=0;
    bool down=0;
    int pos_i=0;
    int pos_j=0;
    std::cout<<"create start"<<std::endl;
    while(1){ 
        //std::cout<<"c"<<std::endl;
        std::string str(buffer);
        if(str.length()<77){
            buffer[0]='m';
            buffer[1]='\n';
            write(sockfd, buffer, MAX);
            bzero(buffer, MAX);
            system("sleep 0.1s");
            read(sockfd, buffer, MAX);
            //printf("%s", buffer);
            if ((strncmp(buffer, "exit", 4)) == 0) {
                printf("Client Exit...\n");
                break;
            }
            std::string temp(buffer);
            str=temp;
        }
        int i=pos_i-1;
        int j=pos_j;
        for (int t=0;t<str.length();t++) {
            char c =str[t];
            if(c=='\n'){
                j = pos_j;
                i++;
                continue;
            }
            if (c != '#' && c != '.' && c != '*' && c != 'E')
                continue;
            if(c=='E'){
                if(str[t+1]=='n'){
                    break;
                }
                
            }
            if(i>100){
                break;
            }
            if (j == pos_j+11) {
                std::cout<<std::endl;
                j = pos_j;
                i++;
            }
            if (i == pos_i+7)
                break;
            
            maze[i][j] = c;
            //std::cout<<maze[i][j];
            j++;
    }
        if(l_r==0){
            //std::cout<<"right"<<std::endl;
            pos_j+=11;
            bzero(buffer, MAX);
            for(int t=0;t<11;t++){
                buffer[t]='l';
            }
            buffer[11]='\n';
            write(sockfd, buffer, MAX);
            bzero(buffer, MAX);
            system("sleep 0.1s");
            read(sockfd, buffer, MAX);
            //printf("%s", buffer);
            if ((strncmp(buffer, "exit", 4)) == 0) {
                printf("Client Exit...\n");
                break;
            }
            if(pos_j>90){
                down=1;
                l_r=2;
            }
            //std::cout<<"pos_j "<<pos_j<<std::endl;
            //std::cout<<"pos_i "<<pos_i<<std::endl;
        }else if(l_r==1){
            //std::cout<<"left"<<std::endl;
            pos_j-=11;
            bzero(buffer, MAX);
            for(int t=0;t<11;t++){
                buffer[t]='j';
            }
            buffer[11]='\n';
            write(sockfd, buffer, MAX);
            bzero(buffer, MAX);
            system("sleep 0.2s");
            read(sockfd, buffer, MAX);
            //printf("%s", buffer);
            if ((strncmp(buffer, "exit", 4)) == 0) {
                printf("Client Exit...\n");
                break;
            }
            if(pos_j<=0){
                down=0;
                l_r=2;
            }
            //std::cout<<"pos_j "<<pos_j<<std::endl;
            //std::cout<<"pos_z "<<pos_i<<std::endl;
            
        }else if(l_r==2){
            //std::cout<<"down"<<std::endl;
            bzero(buffer, MAX);
            pos_i+=7;
            for(int t=0;t<7;t++){
                buffer[t]='k';
            }
            buffer[7]='\n';
            write(sockfd, buffer, MAX);
            bzero(buffer, MAX);
            system("sleep 0.1s");
            read(sockfd, buffer, MAX);
            //printf("%s", buffer);
            if ((strncmp(buffer, "exit", 4)) == 0) {
                printf("Client Exit...\n");
                break;
            }
            if(down==1){
                l_r=1;
            }else{
                l_r=0;
            }
            //std::cout<<"pos_j "<<pos_j<<std::endl;
            //std::cout<<"pos_i "<<pos_i<<std::endl;
        }
        if(pos_i>104){
            //std::cout<<"all done"<<std::endl;
            break;
        }
    }
    return maze;
}
std::vector<std::vector<char>> solve_3(int sockfd,char buffer[], int n) {
    std::vector<std::vector<char>> maze(120, std::vector<char>(120, ' '));
    bool fund=0;
    //to right
    while(fund==0){
        int k=0;
        //std::cout<<"start "<<std::endl;
        std::string str(buffer);
        for(char c : str){
            if(c==' '){
                k+=1;
            }else if(c=='y'){
                break;
            }
            if(k>30){
                fund=1;
            }
        }
        //std::cout<<"k "<<k;
        if(fund==0){
            bzero(buffer, MAX);
            n = 0;
            buffer[0]='j';
            buffer[1]='\n';
            write(sockfd, buffer, MAX);
            bzero(buffer, MAX);
            system("sleep 0.1s");
            read(sockfd, buffer, MAX);
            //printf("%s", buffer);
            if ((strncmp(buffer, "exit", 4)) == 0) {
                printf("Client Exit...\n");
                break;
            }
        }
    }
    bzero(buffer, MAX);
    n = 0;
    buffer[0]='l';
    buffer[1]='\n';
    write(sockfd, buffer, MAX);
    bzero(buffer, MAX);
    system("sleep 0.25s");
    read(sockfd, buffer, MAX);
    //printf("%s", buffer);
    if ((strncmp(buffer, "exit", 4)) == 0) {
        printf("Client Exit...\n");
    }
    fund=0;
    //to up
    while(fund==0){ 
        int k=0;
        std::string str(buffer);
        if (str.find(" 0:") != std::string::npos){
            fund=1;
        }
        if(fund==0){
            bzero(buffer, MAX);
            n = 0;
            buffer[0]='i';
            buffer[1]='\n';
            write(sockfd, buffer, MAX);
            bzero(buffer, MAX);
            system("sleep 0.1s");
            read(sockfd, buffer, MAX);
            //printf("%s", buffer);
            if ((strncmp(buffer, "exit", 4)) == 0) {
                printf("Client Exit...\n");
                break;
            }
        }
    }
    fund=0;
    //to map
    std::cout<<"create maze "<<std::endl;
    maze=create_maze_3(sockfd,buffer, MAX);
    return maze;
}
std::vector<std::vector<char>> create_maze_4(std::string str) {
	std::vector<std::vector<char>> maze(7, std::vector<char>(11, ' '));
	for (int i = 0; i < 7; ++i) {
		unsigned int pos = str.find(": ");
		for (int j = 0; j < 11; ++j)
			maze[i][j] = str[pos + 2 + j];
		if (str.length() < 13)
			break;
		str = str.substr(pos + 13, str.size() - pos - 13 + 1);
	}
	return maze;
}
char get_direction(std::vector<std::vector<char>> maze, char direction, bool& err) {
	point start;
	start.x = -1;
	start.y = -1;
	for (int i = 0; i < maze.size(); ++i) {
		for (int j = 0; j < maze[0].size(); ++j) {
			//std::cout << maze[i][j];
			if (maze[i][j] == '*') {
				start.x = i;
				start.y = j;
			}
		}
		//std::cout << std::endl;
	}
	if (start.x == -1 && start.y == -1) {
		err = true;
		return direction;
	}
	//std::cout << start.x << " " << start.y << std::endl;
	switch (direction) {
		case LEFT :
			if (start.x - 1 >= 0 && maze[start.x - 1][start.y] == '.'  || maze[start.x - 1][start.y] == 'E') 
				return UP;
			if (start.y - 1 >= 0 && maze[start.x][start.y - 1] == '.' || maze[start.x][start.y - 1] == 'E')
				return LEFT;
			if (start.x + 1 < maze.size() && maze[start.x + 1][start.y] == '.' || maze[start.x + 1][start.y] == 'E')
				return DOWN;
			if (start.y + 1 < maze[0].size() && maze[start.x][start.y + 1] == '.' || maze[start.x][start.y + 1] == 'E')
				return RIGHT;
			break;
		case RIGHT :
			if (start.x + 1 < maze.size() && maze[start.x + 1][start.y] == '.' || maze[start.x + 1][start.y] == 'E')
				return DOWN;
			if (start.y + 1 < maze[0].size() && maze[start.x][start.y + 1] == '.' || maze[start.x][start.y + 1] == 'E')
				return RIGHT;
			if (start.x - 1 >= 0 && maze[start.x - 1][start.y] == '.' || maze[start.x - 1][start.y] == 'E') 
				return UP;
			if (start.y - 1 >= 0 && maze[start.x][start.y - 1] == '.' || maze[start.x][start.y - 1] == 'E')
				return LEFT;
			break;
		case UP :
			if (start.y + 1 < maze[0].size() && maze[start.x][start.y + 1] == '.' || maze[start.x][start.y + 1] == 'E')
				return RIGHT;
			if (start.x - 1 >= 0 && maze[start.x - 1][start.y] == '.' ||maze[start.x - 1][start.y] == 'E') 
				return UP;
			if (start.y - 1 >= 0 && maze[start.x][start.y - 1] == '.' || maze[start.x][start.y - 1] == 'E')
				return LEFT;
			if (start.x + 1 < maze.size() && maze[start.x + 1][start.y] == '.' || maze[start.x + 1][start.y] == 'E')
				return DOWN;
			break;
		case DOWN :
			if (start.y - 1	>= 0 && maze[start.x][start.y - 1] == '.' || maze[start.x][start.y - 1] == 'E')
				return LEFT;
			if (start.x + 1 < maze.size() && maze[start.x + 1][start.y] == '.' || maze[start.x + 1][start.y] == 'E')
				return DOWN;
			if (start.y + 1 < maze[0].size() && maze[start.x][start.y + 1] == '.' || maze[start.x][start.y + 1] == 'E')
				return RIGHT;
			if (start.x - 1 >= 0 && maze[start.x - 1][start.y] == '.' || maze[start.x - 1][start.y] == 'E') 
				return UP;
			break;
		default :
			return ' ';
	}
	return ' ';
}
char init_direction(std::vector<std::vector<char>> maze) {
	point start;
	for (int i = 0; i < maze.size(); ++i) {
		for (int j = 0; j < maze[0].size(); ++j) {
			if (maze[i][j] == '*') {
				start.x = i;
				start.y = j;
			}
		}
		//std::cout << std::endl;
	}
	//std::cout << start.x << " " << start.y << std::endl;
	if (start.x > 1 && maze[start.x - 1][start.y] == '.') 
		return UP;
	if (start.x < maze.size() - 1 && maze[start.x + 1][start.y] == '.') 
		return DOWN;
	if (start.y > 1 && maze[start.x][start.y - 1] == '.') 
		return LEFT;
	if (start.y < maze[0].size() - 1 && maze[start.x][start.y + 1] == '.') 
		return RIGHT;
	return ' ';
}
std::string solve_4(int sockfd) {
	std::vector<std::vector<char>> maze(7, std::vector<char>(11, ' '));
	std::string result;
	char buffer[MAX];
	read(sockfd, buffer, sizeof(buffer));
	printf("%s", buffer);
	bzero(buffer, sizeof(buffer));
	buffer[0] = 'm';
	buffer[1] = '\n';
	write(sockfd, buffer, sizeof(buffer));
	system("sleep 0.1s");
	read(sockfd, buffer, sizeof(buffer));
	std::string str(buffer);
	maze = create_maze_4(str);
	char direction = init_direction(maze);
	bool err = false;
	while (true) {
		std::string str(buffer);
		maze = create_maze_4(str);
		direction = get_direction(maze, direction, err);
		bzero(buffer, sizeof(buffer));
		if (err) {
			buffer[0] = 'm';
			err = false;
		}
		else 
			buffer[0] = direction;
		buffer[1] = '\n';
		//bzero(buffer, sizeof(buffer));
		// n = 0;
		// while ((buffer[n++] = getchar()) != '\n')
		// 	;
		write(sockfd, buffer, sizeof(buffer));
		printf("%s", buffer);
		//write(sockfd, buffer, sizeof(buffer));
		bzero(buffer, sizeof(buffer));
        system("sleep 0.15s");
		read(sockfd, buffer, sizeof(buffer));
		// std::string input_buf(buffer);
		// while (input_buf.length() < 30) {
		// 	bzero(buffer, sizeof(buffer));
		// 	system("sleep 0.1s");
		// 	read(sockfd, buffer, sizeof(buffer));
		// 	std::string input_buf(buffer);
		// }
		//read(sockfd, buffer, sizeof(buffer));
		printf("%s", buffer);
	}
	return result;
}
void func(int sockfd) {
    if (port == 10304) {
		std::string result = solve_4(sockfd);
		return;
	}
	char buff[MAX];
	int n;
    bool fund=0;
    std::string answer;
    read(sockfd, buff, sizeof(buff));
	printf("%s", buff);
	if ((strncmp(buff, "exit", 4)) == 0) {
		printf("Client Exit...\n");
	}
		bzero(buff, sizeof(buff));
		n = 0;
        buff[0] = 'm';
        buff[1] = '\n';
		write(sockfd, buff, sizeof(buff));
		bzero(buff, sizeof(buff));
		if ((strncmp(buff, "exit", 4)) == 0) {
			printf("Client Exit...\n");
			return;
		}
        system("sleep 0.25s");
        read(sockfd, buff, sizeof(buff));
		//printf("%s", buff);
        std::vector<std::vector<char>> maze;
        switch(port) {
            case 10301:
                maze = create_maze_1(buff, sizeof(buff));
				for (std::vector<char> v : maze) {
					for (char c : v)
						std::cout << c;
					std::cout << std::endl;
				}
                answer=bfs(maze);
                fund=1;
                break;
            case 10302:
                maze = create_maze_2(buff, sizeof(buff));
                std::cout<<"origin maze "<<std::endl;
                for ( int i =0 ; i < 21; ++i) {
                    for ( int j=0; j < 79; ++j)
                        std::cout<<maze[i][j];
                    std::cout<<std::endl;
                }
                answer=bfs(maze);
                
                fund=1;
                break;
            case 10303:
                maze = solve_3(sockfd,buff, sizeof(buff));
                std::cout<<"orgin maze "<<std::endl;
                for ( int i =0 ; i < 101; ++i) {
                    for ( int j=0; j < 101; ++j)
                        std::cout<<maze[i][j];
                    std::cout<<std::endl;
                }
                answer=bfs(maze);
				
                fund=1;
                break;
        }
        if(answer.length()>0){
            char ans[MAX];
            for(int i=0;i<answer.length();i++){
                ans[i]=answer[i];
            }
            ans[answer.length()]='\n';
            write(sockfd, ans, sizeof(ans));
            bzero(ans, sizeof(ans));
            system("sleep 0.5s");
            read(sockfd, ans, sizeof(ans));
            printf("%s", ans);
            buff[0] = '\n';
            buff[1] = '\0';
            write(sockfd, buff, sizeof(buff));
            read(sockfd, ans, sizeof(ans));
            printf("%s", ans);
            for (int i = 0; i < 65536; ++i) {
                printf("%c", ans[i]);
            }
        }
	
}

int main()
{
	int sockfd, connfd;
	struct sockaddr_in servaddr, cli;
    std::string input;
    getline(std::cin, input);
    port = 10300 + std::stoi(input);
	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("140.113.213.213");
	servaddr.sin_port = htons(port);

	// connect the client socket to server socket
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr))
		!= 0) {
		printf("connection with the server failed...\n");
		exit(0);
	}
	else
		printf("connected to the server..\n");
    system("sleep 1s");
	// function for chat
	func(sockfd);

	// close the socket
	close(sockfd);
}