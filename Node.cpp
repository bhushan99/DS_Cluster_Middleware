#include "Node.h"

Node::Node(string ip, string port):ip(ip),port(port) { }

void Node::startUp(){
	thread listenMessage (&Node::receiveMessage,this);
    listenMessage.detach();
    	
	for(int i=0; i<10; i++){
		
		if(port != ports[i]){
			string res = sendMessage(ips[i],ports[i],to_string(IAmUp)+":"+to_string(stoi(port)%10));
			if(res == "error")
				continue;
			else if(res[0] == '2'){
				sentNodes.insert(i+1);
			}
		}
	}
	cout << "size of set is : " <<  sentNodes.size() << endl;

    heartBeat();
	listenMessage.join();
}

void Node::submitJob(string execFileName, string ipFileName){
    
	cout << "Submit job done!! file names are: \n" << execFileName << "\n" << ipFileName << endl;
}

void Node::heartBeat(){
    while(1){
        sleep(HeartBeatTime);
        cout << "HeartBeat sending started \n";
        set<int>::iterator it;
        for(it = sentNodes.begin(); it != sentNodes.end(); it++){
            int curNode = *it;
            string res = sendMessage("127.0.0.1", to_string(12340+curNode), to_string(CheckAlive)+":"+to_string(curNode));
            if(res == "timeout"){
                cout << sentNodes.size() << endl;
                // call submitJob function to submit required job.
            }
        }
    }

}

string Node::sendMessage(string ip, string port, string msg){

	int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    // struct hostent *server;

    char buffer[256];
    portno = atoi(port.c_str());
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        perror("ERROR opening socket");
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    serv_addr.sin_port = htons(portno);
    cout << "Connecting to " << stoi(port)%10 << endl;
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
        perror("ERROR connecting");
        return "timeout";
    }
    bzero(buffer,256);
    strcpy(buffer,msg.c_str());
    cout << "Sending message " << msg << " to " << ip << endl;
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
        perror("ERROR writing to socket");
    bzero(buffer,256);
    // wait for sometime in this read function. If dont get any read then just break it.
    n = read(sockfd,buffer,255);
    
    if (n < 0) 
        perror("ERROR reading from socket");
    cout << "Got message " << buffer << " from " << ip << endl;
    string ret(buffer);
    close(sockfd);
	return buffer;
}



void Node::receiveMessage(){
	int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
    char buffer1[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
	    perror("Server Socket ");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(port.c_str());
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
    	perror("bind()");
    while(1){
	    listen(sockfd,5);
	    clilen = sizeof(cli_addr);
	    cout << "Will accept now" << endl;
	    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	    if (newsockfd < 0) 
	        perror("accept()");
	    bzero(buffer,256);
	    bzero(buffer1,256);
	    n = read(newsockfd,buffer,255);
	    if (n < 0) perror("ERROR reading from socket");
	    if(buffer[0] == IAmUp+'0'){
	    	int temp = buffer[2] - '0';
	    	sentNodes.insert(temp);   
	    	sprintf(buffer1,"%d:",ReplyAlive);
	    }
	    cout << "size of set is " << sentNodes.size() << endl;
	    n = write(newsockfd,buffer1,256);
	    if (n < 0) perror("ERROR writing to socket");
	}
    
    // close(newsockfd);
    // close(sockfd);
}

string Node::getIp(){
	return this->ip;
}

string Node::getPort(){
	return this->port;
}