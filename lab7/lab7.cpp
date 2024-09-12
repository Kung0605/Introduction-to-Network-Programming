#include <cstdio>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <vector>
#include <iostream>
#define SOCKET_PATH "/queen.sock"
using namespace std;
int board[30][30];
void send_answer(int sockfd){
    char buffer[4096];
    for(int i = 0; i < 30; i++){
        for(int j = 0; j < 30; j++){
            string s="M ";
            if(board[i][j]==1){
                int temp=i/10;
                if(temp>0){
                    s+=char(temp+48);
                }
                s+=char((i%10)+48);
                s+=' ';
                temp=j/10;
                if(temp>0){
                    s+=char(temp+48);
                }
                s+=char((j%10)+48);
                strcpy(buffer, s.c_str());
                if (write(sockfd, buffer, strlen(buffer)) < 0) {
                    perror("Error writing to socket");
                }else{
                    int bytes_received = read(sockfd, buffer, sizeof(buffer));
                    if (bytes_received <= 0) {
                        perror("Error reading from socket");
                    }else{
                        //cout<<"buffer "<<buffer<<endl;
                    }
                }
            }
        }
    }
    string p="P";
    strcpy(buffer, p.c_str());
    if (write(sockfd, buffer, strlen(buffer)) < 0) {
        perror("Error writing to socket");
    }else{
        int bytes_received = read(sockfd, buffer, sizeof(buffer));
        if (bytes_received <= 0) {
            perror("Error reading from socket");
        }else{
            //cout<<"buffer "<<buffer<<endl;
        }
    }
    p="C";
    strcpy(buffer, p.c_str());
    if (write(sockfd, buffer, strlen(buffer)) < 0) {
        perror("Error writing to socket");
    }else{
        int bytes_received = read(sockfd, buffer, sizeof(buffer));
        if (bytes_received <= 0) {
            perror("Error reading from socket");
        }else{
            cout<<"buffer "<<buffer<<endl;
        }
    }
    
}

bool check(int row,int col,vector<string>& board){
    //col
    for(int i=0;i<board.size();++i){
        if(board[row][i]=='Q'){
            //cout<<row<<" "<<i<<" have Q"<<endl;
            return false;
        }
    }
    //row
    for(int i=0;i<board.size();++i){
        if(board[i][col]=='Q'){
            //cout<<i<<" "<<col<<" have Q"<<endl;
            return false;
        }
    }
    //right down
    for(int i=0;i<board.size();++i){
        if(row+i>=board.size()||col+i>=board.size()){
            break;
        }
        if(board[row+i][col+i]=='Q'){
            //cout<<row+i<<" "<<col+i<<" have Q"<<endl;
            return false;
        }
    }
    //left up
    for(int i=0;i<board.size();++i){
        if(row-i<0||col-i<0){
            break;
        }
        if(board[row-i][col-i]=='Q'){
            //cout<<row-i<<" "<<col-i<<" have Q"<<endl;
            return false;
        }
    }
    //left down
    for(int i=0;i<board.size();++i){
        if(row+i>=board.size()||col-i<0){
            break;
        }
        if(board[row+i][col-i]=='Q'){
            //cout<<row+i<<" "<<col-i<<" have Q"<<endl;
            return false;
        }
    }
    //right up
    for(int i=0;i<board.size();++i){
        if(row+i>=board.size()||col-i<0){
            break;
        }
        if(board[row+i][col-i]=='Q'){
            //cout<<row+i<<" "<<col-i<<" have Q"<<endl;
            return false;
        }
    }
    return true;
}
bool solveNQ(vector<string>& board)
{   int start_row=0;
int diff;
    for(int i=0;i<15;i++){
        diff=0;
        for(int j=start_row;j<board.size();j++){
            for(int k=0;k<board.size();k++){
                if(check(j,k,board)){
                    cout<<"j "<<j<<"k "<<k<<endl;
                    board[j][k]='Q';
                    start_row=j+1;
                    diff=1;
                    break;
                }
            }
            if(diff==1){
                break;
            }
        }
    }
    
    return true;
}

bool isValid(int row, int col) {
    for (int i = 0; i < 30; i++)
        if (board[row][i])
            return false;

    for (int i = 0; i < 30; i++)
        if (board[i][col])
            return false;

    for (int i=row, j=col; i>=0 && j>=0; i--, j--)
        if (board[i][j])
            return false;
    for (int i=row, j=col; i<30 && j<30;   i++, j++)
        if (board[i][j])
            return false;
    for (int i=row, j=col; j>=0 && i<30;  i++, j--)
        if (board[i][j])
            return false;
    for (int i=row, j=col; i>=0 && j<30;  i--, j++)
        if (board[i][j])
            return false;
    return true;
}

bool solveNQueen(int col) {
    if (col >= 30)
        return true;

    for (int i = 0; i < 30; i++) {
        if(board[i][col])
            return solveNQueen(col + 1);

        else if (isValid(i, col) ) {
            board[i][col] = 1;
            if(solveNQueen(col + 1))
                return true;
            board[i][col] = 0;
        }
    }
    return false;
}

bool checkSolution() {
    if(solveNQueen(0) == false) {
        cout << "Solution does not exist";
        return false;
    }

    return true;
}
void communicateWithServer(int sockfd) {
    char buffer[4096];
    size_t bytesRead;
    buffer[0] = 'P';
    if (write(sockfd, buffer, strlen(buffer)) < 0) {
            perror("Error writing to socket");
    }else{
        //printf("Write to socket\n");
    }
    int bytes_received = read(sockfd, buffer, sizeof(buffer));
    if (bytes_received <= 0) {
        perror("Error reading from socket");
    }else{
        //printf("Read from socket\n");
    }
    //printf("Received %d \n %s\n", bytes_received, buffer);
    vector<string> v;
    for(int i = 0; i < bytes_received; i++){
        string s = "";
        while(buffer[i] != '\n'){
            if(buffer[i]=='Q'||buffer[i]=='.'){
                s += buffer[i];
            }
             i++;
        }
        v.push_back(s);
    }
    for(int i=0;i<30;i++){
        for(int j=0;j<30;j++){
            if(v[i][j]=='Q'){
                board[i][j]=1;
            }else if(v[i][j]=='.'){
                board[i][j]=0;
            }else{
                //board[i][j]=2;
            }
            //cout<<board[i][j]<<" ";
        }
        //cout<<endl;
    }
    if(checkSolution() == false){
        //cout << "SOLVER ERR: INCOMPLETE!\n";
    }else{
        //cout<<"SOLVER OK: COMPLETE!\n";
    }
    for(int i=0;i<30;i++){
        for(int j=0;j<30;j++){
            //cout<<board[i][j]<<" ";
        }
        //cout<<endl;
    }
    send_answer(sockfd);
}

int main() {
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error opening socket");
        exit(EXIT_FAILURE);
    }else{
        //printf("Socket opened\n");
    }
    struct sockaddr_un server_addr;
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SOCKET_PATH);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error connecting to server");
        close(sockfd);
        exit(EXIT_FAILURE);
    }else{
        //printf("Connected to server\n");
    }

    // Communicate with the server
    communicateWithServer(sockfd);

    // Close the socket
    close(sockfd);

    return 0;
}