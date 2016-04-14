#include "Node.h"

Node::Node(string ip, string port):ip(ip),port(port) { ID=ip+string(":")+port; }

void Node::startUp(){
	thread listenMessage (&Node::receiveMessage,this);
    listenMessage.detach();
    	
	for(int i=0; i<10; i++){
		
		if(port != ports[i]){
			string res = sendMessage(ips[i],ports[i],to_string(IAmUp)+"::"+ip+":"+port);
			if(res == "error")
				continue;
			else if(res[0] == '2'){
                int idx = res.find("::");
				sentNodes.insert(res.substr(idx+2));
			}
		}
	}
	cout << "size of set is : " <<  sentNodes.size() << endl;

    thread heartBeatMessage (&Node::heartBeat,this);
    heartBeatMessage.detach();
	// listenMessage.join();
}

bool cmp(const pair<string,int>& p1,const pair<string,int>& p2) {
    return p1.second<p2.second;
}

void getDestNodes(vector<pair <string,int> >& load) {
    sort(load.begin(),load.end(),cmp);
    int mx=0;
    for(int i=0;i<load.size()-1;i++) mx=max(mx,load[i+1].second-load[i].second);
    if(!mx) return;
    for(int i=0;i<load.size()-1;i++) {
        if(load[i+1].second-load[i].second==mx) {
            load=vector<pair<string,int> >(load.begin(),load.begin()+i+1);
            return;
        }
    }
} 

pair<string,string> split(string NodeID) {
    string a="";int i;
    for(i=0;NodeID[i]!=':';i++) a+=NodeID[i];
    return pair<string,string>(a,NodeID.substr(i+1));
}

void Node::submitJob(string execFileName, string ipFileName){
    MD5 md5;
    string exf(md5.digestFile(execFileName.c_str()));
    string ipf(md5.digestFile(ipFileName.c_str()));
    Job j;
    j.execFile=exf;
    j.ipFile=ipf;
    j.JobId=exf+string(":")+ipf;
    j.ownerId=this->ID;
    md5_original[j.JobId]=execFile+string(":")+ipFileName;
    localQ.push(j);
    load.erase(load.begin(),load.end());

    // SEND load query to all nodes, receive reply, fill load

    getDestNodes(load);
    Application app;
    vector<Job> vj= app.split(j,load.size()+1);
    for(int i=0;i<vj.size()-1;i++) //send files to nodes in load[i]
    {
        sentNodes.insert(load[i].first);
        nodeToJob[load[i].first].insert(vj[i].JobId);
        inputMapping[j.JobId].insert(pair<string,int>(load[i].first,i+1));
    }
    globalQ.push(vj[vj.size()-1]); //keep self part

	

    cout << "Submit job done!! file names are: \n" << execFileName << "\n" << ipFileName << endl;
}

void Node::heartBeat(){
    while(1){
        sleep(HeartBeatTime);
        cout << "HeartBeat sending started \n";
        set<string>::iterator it;
        for(it = sentNodes.begin(); it != sentNodes.end(); it++){
            string curNode = *it;
            pair<string,string> p=split(curNode);
            string res = sendMessage(p.first,p.second,to_string(CheckAlive)+"::"+curNode);
            if(res == "timeout"){
                cout << curNode << " is not alive!!" << endl;
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
    struct timeval tv;

    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;


    char buffer[256];
    portno = atoi(port.c_str());
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        perror("ERROR opening socket");
    
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    serv_addr.sin_port = htons(portno);
    cout << "Connecting to " << ip+":"+port << endl;
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
        perror("ERROR connecting");
        return "timeout";
    }
    bzero(buffer,256);
    strcpy(buffer,msg.c_str());
    cout << "Sending message " << msg << " to " << ip + ":" + port << endl;
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
        perror("ERROR writing to socket");
    bzero(buffer,256);
    // wait for sometime in this read function. If dont get any read then just break it.
    n = read(sockfd,buffer,255);
    
    if(errno == EAGAIN || errno == EWOULDBLOCK)
    {
        return "timeout";
    }    
    if (n < 0) 
        perror("ERROR reading from socket");
    cout << "Got message " << buffer << " from " << ip + ":" + port << endl;
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
            string str(buffer);
            int idx = str.find("::");
	    	sentNodes.insert(str.substr(idx+2));   
	    	sprintf(buffer1,"%d::",ReplyAlive);
            strcat(buffer1,ip.c_str());
            strcat(buffer1,(":"+port).c_str());
	    }
        else if(buffer[0] == JobSend + '0' || buffer[0] == InputSend + '0'){
            string fileName;
            bool isLast;
            string str(buffer);
            int idx = str.find("::");
            int idx1 = str.find(":",idx+2);
            int idx2 = str.find(":",idx1+1);
            fileName = str.substr(idx1+1,idx2-idx1-1);
            isLast = ((str.substr(idx+2,idx1-idx-2) == "0")? 0 : 1);
            string message = str.substr(idx2+1);
            char mes[256];
            strcpy(mes,message.c_str());
            FILE *fp = fopen(fileName.c_str(),"a");
            fwrite(mes,sizeof(char),sizeof(mes),fp);
            fclose(fp); 
        }
	    cout << "size of set is " << sentNodes.size() << endl;
	    n = write(newsockfd,buffer1,256);
	    if (n < 0) perror("ERROR writing to socket");
	}
    
    // close(newsockfd);
    // close(sockfd);
}

string Node::sendFile(string ip, string port, string fileName, int type){
    int fileNameSize = fileName.size();
    int maxsize = MAX - 16 - fileNameSize, lastMessage = 0;
    int constantMessage;

    if(type == 1)
        constantMessage = InputSend;
    else
        constantMessage = JobSend;

    char *tempBuffer=(char*)malloc(MAX*sizeof(char)),*buffer=(char*)malloc(MAX*sizeof(char)); 
    FILE *fp=fopen(fileName.c_str(),"r");

    fseek(fp,0,SEEK_END);
    int f_sz=ftell(fp);
    rewind(fp);

    int size=0;
    int nbytes=min(f_sz,maxsize);
    
    while((size=fread(tempBuffer,sizeof(char),nbytes,fp))>0){
        if(size < maxsize)
            lastMessage = 1;

        sprintf(buffer, "%d::%d:%s:%s", constantMessage, lastMessage, fileName.c_str(), tempBuffer);
        Node::sendMessage(ip, port, buffer);

        free(buffer);
        free(tempBuffer);
        buffer=(char*)malloc(MAX*sizeof(char));
        tempBuffer=(char*)malloc(MAX*sizeof(char));
        memset(buffer,0,MAX);
        memset(tempBuffer,0,MAX);

        f_sz-=size;
        nbytes=min(f_sz,maxsize);
    }   
    fclose(fp);
    cout<<" file sent\n";
}

void receiveFile(){

}

string Node::getIp(){
	return this->ip;
}

string Node::getPort(){
	return this->port;
}