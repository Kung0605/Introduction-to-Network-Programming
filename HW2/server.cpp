#include <iostream>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include <arpa/inet.h>
#include <unordered_map>
#include <algorithm>
#include <sys/socket.h>
#include <sys/types.h>
#define MAX_CLIENTS 16384
#define BUFFER_SIZE 4096
using namespace std;
fd_set readfds;
std::vector<int> clients(MAX_CLIENTS, 0);
unordered_map<string, string> userMap;//username,password
unordered_map<int,string> loginMap;//socket,username
unordered_map<string,int> statusMap;//username,0:offline, 1:online, 2:busy
unordered_map<int,vector<string>> chatRoomunorderedMap;//room number, user list
unordered_map<int,int>whoinchatroom;//socket,room number
unordered_map<int,vector<pair<string,string>>> chatroomhistory;//room number, chat history
string chatroomowner[101];//room number, owner
unordered_map<int,string> pinmessange;//room number, message
unordered_map<int,vector<string>> chatRoomMap;//room number, user list
void sendWelcomeMessage(int clientSocket) {
    const char* welcomeMessage = "*********************************\n"
                                 "** Welcome to the Chat server. **\n"
                                 "*********************************\n"
                                 "% ";
    send(clientSocket, welcomeMessage, strlen(welcomeMessage), 0);
}
bool processCommand(int senderSocket, const char* command) {

    if (strncmp(command, "register", 8) == 0) {

        char username[20];
        char password[20];

        if (sscanf(command, "register %s %s", username, password) == 2) {
            // Check parameter lengths
            if (strlen(username) >= 20 || strlen(password) >= 20) {
                const char* usageMessage = "Usage: /register <username> <password>\n"
                                           "Both username and password must be less than 20 characters.\n";
                send(senderSocket, usageMessage, strlen(usageMessage), 0);
            } else {
                if (userMap.find(username) != userMap.end()) {
                    const char* usernameUsedMessage = "Username is already used.\n";
                    send(senderSocket, usernameUsedMessage, strlen(usernameUsedMessage), 0);
                } else {

                    const char* registerSuccessMessage = "Register successfully.\n";
                    send(senderSocket, registerSuccessMessage, strlen(registerSuccessMessage), 0);

                    userMap.insert(pair<string, string>(username, password));
                    statusMap.insert(pair<string,int>(username,0));
                }
            }
        } else {
            const char* usageMessage = "Usage: register <username> <password>\n";
            send(senderSocket, usageMessage, strlen(usageMessage), 0);
        }
    } else if(strncmp(command, "login", 5) == 0){
        char username[20];
        char password[20];

        if (sscanf(command, "login %s %s", username, password) == 2) {
            // Check parameter lengths
            if (strlen(username) >= 20 || strlen(password) >= 20) {
                const char* usageMessage = "Usage: /login <username> <password>\n"
                                           "Both username and password must be less than 20 characters.\n";
                send(senderSocket, usageMessage, strlen(usageMessage), 0);
            } else {
                if (userMap.find(username) == userMap.end()) {
                    const char* usernameUsedMessage = "Login failed.\n";
                    send(senderSocket, usernameUsedMessage, strlen(usernameUsedMessage), 0);
                } else {
                    if(userMap[username] == password){
                        if(loginMap.find(senderSocket) != loginMap.end()){
                            const char* loginFailedMessage = "Please logout first.\n";
                            send(senderSocket, loginFailedMessage, strlen(loginFailedMessage), 0);
                            return false;
                        }
                        string temp = username;
                        string messages = "Welcome, "+temp+".\n";
                        send(senderSocket, messages.c_str(), messages.length(), 0);
                        loginMap.insert(pair<int,string>(senderSocket,username));
                        //cout<<"sendersocket:"<<senderSocket<<" username: "<<username<<endl;
                        //cout<<"loginmap:"<<loginMap[senderSocket]<<endl;
                        statusMap[username] = 1;
                        return true;
                    }else{
                        const char* loginFailedMessage = "Login failed.\n";
                        send(senderSocket, loginFailedMessage, strlen(loginFailedMessage), 0);
                    }
                }
            }
        } 
        else {
            const char* usageMessage = "Usage: login <username> <password>\n";
            send(senderSocket, usageMessage, strlen(usageMessage), 0);
        }
    }else if(strncmp(command,"logout",6) == 0){
        if(loginMap.find(senderSocket) == loginMap.end()){
            const char* logoutFailedMessage = "Please login first.\n";
            send(senderSocket, logoutFailedMessage, strlen(logoutFailedMessage), 0);
            return false;
        }
        string messages = "Bye, "+loginMap[senderSocket] + ".\n";
        const char* logoutSuccessMessage = messages.c_str();
        send(senderSocket, logoutSuccessMessage, strlen(logoutSuccessMessage), 0);
        //cout<<"logout: "<<loginMap[senderSocket]<<endl;
        statusMap[loginMap[senderSocket]] = 0;
        loginMap.erase(senderSocket);
        
        return true;
    }else if(strncmp(command,"whoami",6) == 0){
        if(loginMap.find(senderSocket) == loginMap.end()){
            const char* whoamiFailedMessage = "Please login first\n";
            send(senderSocket, whoamiFailedMessage, strlen(whoamiFailedMessage), 0);
            return false;
        }
        string messages = loginMap[senderSocket] + "\n";
        const char* whoamiSuccessMessage = messages.c_str();
        send(senderSocket, whoamiSuccessMessage, strlen(whoamiSuccessMessage), 0);
    }else if(strncmp(command,"set-status",10) == 0){
        if(loginMap.find(senderSocket) == loginMap.end()){
            const char* setStatusFailedMessage = "Please login first\n";
            send(senderSocket, setStatusFailedMessage, strlen(setStatusFailedMessage), 0);
            return false;
        }
        char status[50];
        if(sscanf(command, "set-status %s", status) == 1){
            if(strcmp(status,"online") == 0){
                statusMap[loginMap[senderSocket]] = 1;
                string messages = loginMap[senderSocket]+" online\n";
                const char* setStatusSuccessMessage = messages.c_str();
                send(senderSocket, setStatusSuccessMessage, strlen(setStatusSuccessMessage), 0);
            }else if(strcmp(status,"offline") == 0){
                statusMap[loginMap[senderSocket]] = 0;
                string messages = loginMap[senderSocket]+" offline\n";
                const char* setStatusSuccessMessage = messages.c_str();
                send(senderSocket, setStatusSuccessMessage, strlen(setStatusSuccessMessage), 0);
            }else if(strcmp(status,"busy") == 0){
                statusMap[loginMap[senderSocket]] = 2;
                string messages = loginMap[senderSocket]+" busy\n";
                const char* setStatusSuccessMessage = messages.c_str();
                send(senderSocket, setStatusSuccessMessage, strlen(setStatusSuccessMessage), 0);
            }else{
                const char* setStatusFailedMessage = "set-status failed\n";
                send(senderSocket, setStatusFailedMessage, strlen(setStatusFailedMessage), 0);
            }
        }else{
            const char* setStatusFailedMessage = "set-status failed\n";
            send(senderSocket, setStatusFailedMessage, strlen(setStatusFailedMessage), 0);
        }
    }else if(strncmp(command,"list-user",9) == 0){
        if(loginMap.find(senderSocket) == loginMap.end()){
            const char* listUserFailedMessage = "Please login first\n";
            send(senderSocket, listUserFailedMessage, strlen(listUserFailedMessage), 0);
            return false;
        }
        string tmessages = "";
        vector<string> temp;
        for(auto it = statusMap.begin(); it != statusMap.end(); ++it){
            
            if(it->second == 1){
                tmessages = it->first + " online\n";
            }else if(it->second == 0){
                tmessages = it->first + " offline\n";
            }else if(it->second == 2){
                tmessages = it->first + " busy\n";
            }

            temp.push_back(tmessages);

        }
        
        sort(temp.begin(),temp.end());
        string messages = "";
        for(auto it = temp.begin(); it != temp.end(); ++it){
            messages += *it;
        }
        const char* listUserSuccessMessage = messages.c_str();
        send(senderSocket, listUserSuccessMessage, strlen(listUserSuccessMessage), 0);
    }else if(strncmp(command,"exit",4) == 0&&(whoinchatroom.find(senderSocket) == whoinchatroom.end())){
        if(loginMap.find(senderSocket) != loginMap.end()){
            string messages = "Bye, "+loginMap[senderSocket] + ".\n";
            const char* logoutSuccessMessage = messages.c_str();
            send(senderSocket, logoutSuccessMessage, strlen(logoutSuccessMessage), 0);
            statusMap[loginMap[senderSocket]] = 0;
            loginMap.erase(senderSocket);
        }
        FD_CLR(senderSocket,&readfds);
        for (auto& client : clients) {
            if (client == senderSocket) {
                client = 0;
                break;
            }
        }

        close(senderSocket);
        
        senderSocket = -1;
        return true;
    }else if(strncmp(command,"enter-chat-room",15) == 0){
        if(loginMap.find(senderSocket) == loginMap.end()){
            const char* enterChatRoomFailedMessage = "Please login first\n";
            send(senderSocket, enterChatRoomFailedMessage, strlen(enterChatRoomFailedMessage), 0);
            return false;
        }
        char number[20];
        if(sscanf(command, "enter-chat-room %s", number) == 1){
            int num = atoi(number);
            //if chat romm is not exist
            bool new_room = false;
            if(num > 0 && num <= 100){
                if(chatRoomMap.find(num) == chatRoomMap.end()){
                    //new room
                    vector<string> temp;
                    temp.push_back(loginMap[senderSocket]);
                    chatRoomMap.insert(pair<int,vector<string>>(num,temp));
                    whoinchatroom.insert(pair<int,int>(senderSocket,num));
                    new_room = true;
                    chatroomowner[num]=loginMap[senderSocket];
                }else{
                    chatRoomMap[num].push_back(loginMap[senderSocket]);
                    whoinchatroom.insert(pair<int,int>(senderSocket,num));
                }

                string messages = "Welcome to the public chat room.\nRoom number: "+to_string(num)+"\nOwner: "+chatroomowner[num]+"\n";
                if(new_room==0){
                    for(auto it =chatroomhistory[num].begin(); it != chatroomhistory[num].end(); ++it){
                        if(it->second.find("Pin") != string::npos){
                            messages += it->second;
                        }else{
                            messages += "["+it->first + "]: " + it->second;
                        }
                    }
                    if(pinmessange.find(num) != pinmessange.end()){
                        if(pinmessange[num]!=""){
                            messages += pinmessange[num];
                        }
                    }
                }
                send(senderSocket, messages.c_str(), messages.size(), 0);
                //send other user
                string messages2 = loginMap[senderSocket]+" had enter the chat room.\n";
                for(auto it=whoinchatroom.begin();it!=whoinchatroom.end();++it){
                    if(it->second == num && it->first != senderSocket){
                        
                        send(it->first, messages2.c_str(), messages2.size(), 0);
                    }
                }
                
            }else{
                //Number <number> is not valid.
                string messages = "Number "+to_string(num)+" is not valid.\n";
                send(senderSocket, messages.c_str(), messages.size(), 0);
            }
        }else{
            const char* enterChatRoomFailedMessage = "Usage: enter-chat-room <number>\n";
            send(senderSocket, enterChatRoomFailedMessage, strlen(enterChatRoomFailedMessage), 0);
        }
    }else if(strncmp(command,"list-chat-room",14) == 0){
        if(loginMap.find(senderSocket) == loginMap.end()){
            const char* listChatRoomFailedMessage = "Please login first\n";
            send(senderSocket, listChatRoomFailedMessage, strlen(listChatRoomFailedMessage), 0);
            return false;
        }
        string messages = "";
        for(int i=1;i<=100;i++){
            if(chatroomowner[i]!=""){
                messages +=chatroomowner[i] + " " + to_string(i) + "\n";
            }
        }
        send(senderSocket, messages.c_str(), messages.size(), 0);
    }else if(strncmp(command,"close-chat-room",15) == 0){
        if(loginMap.find(senderSocket) == loginMap.end()){
            const char* enterChatRoomFailedMessage = "Please login first\n";
            send(senderSocket, enterChatRoomFailedMessage, strlen(enterChatRoomFailedMessage), 0);
            return false;
        }
        char number[20];
        if(sscanf(command, "close-chat-room %s", number) == 1){
            int num = atoi(number);
            if(num > 0 && num <= 100){
                if(chatRoomMap.find(num) == chatRoomMap.end()){
                    //room not exist
                     string messages = "Chat room "+to_string(num)+" does not exist.\n";
                    send(senderSocket, messages.c_str(), messages.size(), 0);
                    return false;
                }else{
                    if(chatroomowner[num] == loginMap[senderSocket]){
                        
                        string messages = "Chat room "+to_string(num)+" was closed.\n";
                        
                        send(senderSocket, messages.c_str(), messages.size(), 0);
                        //send other user
                        vector<int> temp;
                        messages+="% ";
                        for(auto it=whoinchatroom.begin();it!=whoinchatroom.end();++it){
                            if(it->second == num && it->first != senderSocket){
                                send(it->first, messages.c_str(), messages.size(), 0);
                                
                                temp.push_back(it->first);
                            }
                            
                        }
                        
                        for(auto it = temp.begin(); it != temp.end(); ++it){
                            whoinchatroom.erase(*it);
                        }
                        chatRoomMap.erase(num);
                        chatroomowner[num] = "";
                        whoinchatroom.erase(senderSocket);
                        chatroomhistory[num].clear();
                    }else{
                        //not owner
                        string messages = "Only the owner can close this chat room.\n";
                        send(senderSocket, messages.c_str(), messages.size(), 0);
                    }
                }
            }else{
                //Chat room <number> does not exist.
                string messages = "Chat room "+to_string(num)+" does not exist.\n";
                send(senderSocket, messages.c_str(), messages.size(), 0);
                return false;
            }
        }else{
            const char* enterChatRoomFailedMessage = "Usage: close-chat-room <number>\n";
            send(senderSocket, enterChatRoomFailedMessage, strlen(enterChatRoomFailedMessage), 0);
        }
    }else if(strncmp(command,"/pin",4) == 0){
        if(loginMap.find(senderSocket) == loginMap.end()){
            const char* enterChatRoomFailedMessage = "Please login first\n";
            send(senderSocket, enterChatRoomFailedMessage, strlen(enterChatRoomFailedMessage), 0);
            return false;
        }
        if(whoinchatroom.find(senderSocket) == whoinchatroom.end()){
            string messages="Error: Unknown command";
            send(senderSocket, messages.c_str(), messages.size(), 0);
            return false;
        }
        string name =loginMap[senderSocket];
        string messages(command);
        messages = messages.substr(5);
        if(messages.find("==") != string::npos){
            messages.replace(messages.find("=="),4,"****");
        }
        if(messages.find("Superpie") != string::npos){
            messages.replace(messages.find("Superpie"),8,"********");
        }
        if(messages.find("hello") != string::npos){
            messages.replace(messages.find("hello"),5,"*****");
        }
        if(messages.find("starburst stream") != string::npos){
            messages.replace(messages.find("starburst stream"),16,"****************");
        }
        if(messages.find("Starburst Stream") != string::npos){
            messages.replace(messages.find("Starburst Stream"),16,"****************");
        }
        if(messages.find("Domain Expansion") != string::npos){
            messages.replace(messages.find("Domain Expansion"),16,"****************");
        }
        if(messages.find("domain expansion") != string::npos){
            messages.replace(messages.find("domain expansion"),16,"****************");
        }
        string temp = "Pin -> ["+name+"]: "+messages;

        pinmessange[whoinchatroom[senderSocket]]=temp;
        for(auto it=whoinchatroom.begin();it!=whoinchatroom.end();++it){
            if(it->second == whoinchatroom[senderSocket]){
                send(it->first, temp.c_str(), temp.size(), 0); 
            }
        }
        
    }else if(strncmp(command,"/delete-pin",11) == 0){
        if(loginMap.find(senderSocket) == loginMap.end()){
            const char* enterChatRoomFailedMessage = "Please login first\n";
            send(senderSocket, enterChatRoomFailedMessage, strlen(enterChatRoomFailedMessage), 0);
            return false;
        }
        if(whoinchatroom.find(senderSocket) == whoinchatroom.end()){
            string messages="Error: Unknown command";
            send(senderSocket, messages.c_str(), messages.size(), 0);
            return false;
        }
        if(pinmessange.find(whoinchatroom[senderSocket]) == pinmessange.end()){
            string messages="No pin message in chat room "+to_string(whoinchatroom[senderSocket])+"\n";
            send(senderSocket, messages.c_str(), messages.size(), 0);
            return false;
        }
        pinmessange.erase(whoinchatroom[senderSocket]);
    }else if(strncmp(command,"/exit-chat-room",15) == 0){
        if(loginMap.find(senderSocket) == loginMap.end()){
            const char* enterChatRoomFailedMessage = "Please login first\n";
            send(senderSocket, enterChatRoomFailedMessage, strlen(enterChatRoomFailedMessage), 0);
            return false;
        }
        if(whoinchatroom.find(senderSocket) == whoinchatroom.end()){
            string messages="Error: Unknown command\n";
            send(senderSocket, messages.c_str(), messages.size(), 0);
            return false;
        }
        int roomnumber = whoinchatroom[senderSocket];
        for(auto it = chatRoomMap[roomnumber].begin(); it != chatRoomMap[roomnumber].end(); ++it){
            if(*it == loginMap[senderSocket]){
                chatRoomMap[roomnumber].erase(it);
                break;
            }
        }
        whoinchatroom.erase(senderSocket);
        for(auto it=whoinchatroom.begin();it!=whoinchatroom.end();++it){
            if(it->second == roomnumber && it->first != senderSocket){
                string messages = loginMap[senderSocket]+" had left the chat room.\n";
                send(it->first, messages.c_str(), messages.size(), 0);
            }
        }
    }else if(strncmp(command,"/list-user",10) == 0){
        if(loginMap.find(senderSocket) == loginMap.end()){
            const char* enterChatRoomFailedMessage = "Please login first\n";
            send(senderSocket, enterChatRoomFailedMessage, strlen(enterChatRoomFailedMessage), 0);
            return false;
        }
        if(whoinchatroom.find(senderSocket) == whoinchatroom.end()){
            string messages="Error: Unknown command";
            send(senderSocket, messages.c_str(), messages.size(), 0);
            return false;
        }
        int roomnumber = whoinchatroom[senderSocket];
        string tmessages="";
        vector<string> temp;
        for(auto it = chatRoomMap[roomnumber].begin(); it != chatRoomMap[roomnumber].end(); ++it){

            if(statusMap[*it] == 1){
                tmessages = *it + " online\n";
            }else if(statusMap[*it] == 0){
                tmessages = *it + " offline\n";
            }else if(statusMap[*it] == 3){
                tmessages = *it + " busy\n";
            }
            temp.push_back(tmessages);
        }
        sort(temp.begin(),temp.end());
        string messages = "";
        for(auto it = temp.begin(); it != temp.end(); ++it){
            messages += *it;
        }
        send(senderSocket, messages.c_str(), messages.size(), 0);
    }else if(strncmp(command,"/",1) == 0){
        string messages="Error: Unknown command\n";
        send(senderSocket, messages.c_str(), messages.size(), 0);
    }
    else {
        if(loginMap.find(senderSocket) == loginMap.end()){
            const char* enterChatRoomFailedMessage = "Please login first\n";
            send(senderSocket, enterChatRoomFailedMessage, strlen(enterChatRoomFailedMessage), 0);
            return false;
        }
        if(whoinchatroom.find(senderSocket) == whoinchatroom.end()){
            string messages="Error: Unknown command\n";
            send(senderSocket, messages.c_str(), messages.size(), 0);
            return false;
        }

        int roomnumber = whoinchatroom[senderSocket];
        string name = loginMap[senderSocket];
        string messages = command;
        if(messages.find("==") != string::npos){
            messages.replace(messages.find("=="),4,"****");
        }
        if(messages.find("Superpie") != string::npos){
            messages.replace(messages.find("Superpie"),8,"********");
        }
        if(messages.find("hello") != string::npos){
            messages.replace(messages.find("hello"),5,"*****");
        }
        if(messages.find("Starburst Stream") != string::npos){
            messages.replace(messages.find("Starburst Stream"),16,"****************");
        }
        if(messages.find("starburst stream") != string::npos){
            messages.replace(messages.find("starburst stream"),16,"****************");
        }
        if(messages.find("Domain Expansion") != string::npos){
            messages.replace(messages.find("Domain Expansion"),16,"****************");
        }
        if(messages.find("domain expansion") != string::npos){
            messages.replace(messages.find("domain expansion"),16,"****************");
        }
        string temp = "["+name+"]: "+messages;
        if(chatroomhistory[roomnumber].size() >= 10){
            chatroomhistory[roomnumber].erase(chatroomhistory[roomnumber].begin());
        }
        chatroomhistory[roomnumber].push_back(pair<string,string>(name,messages));
        for(auto it=whoinchatroom.begin();it!=whoinchatroom.end();++it){
            if(it->second == roomnumber){
                send(it->first, temp.c_str(), temp.size(), 0);
            }
        }
    }
    
    return 0;
}
int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " [port number]" << endl;
        return EXIT_FAILURE;
    }

    int port = atoi(argv[1]);  // Convert the command-line argument to an integer
    int serverSocket, newSocket, maxClients, activity, valRead;
    struct sockaddr_in serverAddr;
    

    // Create a socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up the server address struct
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port); // You can change this port number
    int opt=1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    }
    // Bind the socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(serverSocket, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }


    

    while (true) {
        FD_ZERO(&readfds);
        FD_SET(serverSocket, &readfds);
        maxClients = serverSocket;

        for (const auto& client : clients) {
            if (client > 0) {
                FD_SET(client, &readfds);
                if (client > maxClients) {
                    maxClients = client;
                }
            }
        }

        activity = select(maxClients + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            perror("Select error");
        }

        // Check if there is a new incoming connection
        if (FD_ISSET(serverSocket, &readfds)) {
            if ((newSocket = accept(serverSocket, (struct sockaddr*)NULL, NULL)) < 0) {
                perror("Accept error");
                exit(EXIT_FAILURE);
            }

            
            
            for (auto& client : clients) {
                if (client == 0) {
                    client = newSocket;
                    sendWelcomeMessage(newSocket);  // Send welcome message to the new client
                    break;
                }
            }
        }

        // Check data from clients
        for (auto& client : clients) {
            if (FD_ISSET(client, &readfds)) {
                char buffer[BUFFER_SIZE];
                valRead = read(client, buffer, BUFFER_SIZE);

                if (valRead == 0) {
                    // Client disconnected
                    getpeername(client, (struct sockaddr*)&serverAddr, (socklen_t*)&serverAddr);
                    close(client);
                    client = 0;
                } else {
                    buffer[valRead] = '\0';  // Ensure null-terminated string
                    processCommand(client, buffer);
                    if(whoinchatroom.find(client) == whoinchatroom.end()){
                        string messages = "% ";
                        send(client, messages.c_str(), messages.size(), 0);
                    }
                }
            }
        }
    }

    return 0;
}