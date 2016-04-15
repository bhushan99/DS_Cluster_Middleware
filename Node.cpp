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

pair<string,string> split_(string NodeID) {
    string a="";int i;
    for(i=0;NodeID[i]!=':';i++) a+=NodeID[i];
    return pair<string,string>(a,NodeID.substr(i+1));
}

void Node::submitJob(string execFileName, string ipFileName){
    MD5 md5;
    char buf[256];
    strcpy(buf,execFileName.c_str());
    string exf(md5.digestFile(buf));
    strcpy(buf,ipFileName.c_str());
    string ipf(md5.digestFile(buf));
    Job j;
    j.execFile=execFileName;
    j.ipFile=ipFileName;
    j.jobId=exf+string(":")+ipf;
    j.ownerId=this->ID;
    md5_original[j.jobId]=execFileName+string(":")+ipFileName;
    localQ.push_back(j);
    load.erase(load.begin(),load.end());

    // SEND load query to all nodes, receive reply, fill load

    getDestNodes(load);
    Application app;
    vector<Job> vj= app.split(j,load.size()+1);
    for(int i=0;i<vj.size()-1;i++) //send files to nodes in load[i]
    {
        sentNodes.insert(load[i].first);
        nodeToJob[load[i].first].push_back(vj[i]);
        inputMapping[j.jobId].insert(pair<string,int>(load[i].first,i+1));
    }
    globalQ.push_back(vj[vj.size()-1]); //keep self part
    inputMapping[j.jobId].insert(pair<string,int>(ID,vj.size()));
	

    cout << "Submit job done!! file names are: \n" << execFileName << "\n" << ipFileName << endl;
}

void Node::heartBeat(){
    while(1){
        sleep(HeartBeatTime);
        cout << "HeartBeat sending started \n";
        set<string>::iterator it;
        for(it = sentNodes.begin(); it != sentNodes.end(); it++){
            string curNode = *it;
            pair<string,string> p=split_(curNode);
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
    // cout << "Connecting to " << ip+":"+port << endl;
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
        // perror("ERROR connecting");
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

void Node::receive_IamUP(string newnodeid) {
    deque<Job>::iterator it;
    int ind=0;
    for(it=globalQ.begin();it!=globalQ.end();) {
        Job j=*it;
        if(j.ownerId!=ID) {it++;continue;}
        char buf[256];
        MD5 md5;
        strcpy(buf,j.execFile.c_str());
        string exf(md5.digestFile(buf));
        strcpy(buf,j.ipFile.c_str());
        string ipf(md5.digestFile(buf));
        string newjid=exf+string(":")+ipf;
        j.jobId=newjid;
        Application app;
        vector<Job> vj= app.split(j,2);
        for(int i=0;i<vj.size()-1;i++) //send files to nodes in newnodeid
        {
            sentNodes.insert(newnodeid);
            nodeToJob[newnodeid].push_back(vj[i]);
            inputMapping[j.jobId].insert(pair<string,int>(newnodeid,i+1));
        }
        set<pair<string,int> >::iterator z=inputMapping[it->jobId].begin();
        while(z->first!=ID) z++;
        parent[newjid]=pair<string,int>(it->jobId,z->second);
        //inputMapping[it->jobId].erase(z);
        inputMapping[newjid].insert(pair<string,int>(ID,vj.size()));
        globalQ[ind]=vj[vj.size()-1];
        it++;ind++;
    }
}

void Node::nodeFail(string failnodeid) {
    vector<Job>::iterator it;
    if(nodeToJob.find(failnodeid)==nodeToJob.end()) return;
    for(it=nodeToJob[failnodeid].begin();it!=nodeToJob[failnodeid].end();) {
        Job j=*it;
        char buf[256];
        MD5 md5;
        strcpy(buf,j.execFile.c_str());
        string exf(md5.digestFile(buf));
        strcpy(buf,j.ipFile.c_str());
        string ipf(md5.digestFile(buf));
        string newjid=exf+string(":")+ipf;
        set<pair<string,int> >::iterator z=inputMapping[it->jobId].begin();
        while(z->first!=failnodeid) z++;
        parent[newjid]=pair<string,int>(it->jobId,z->second);
        //inputMapping[it->jobId].erase(z);
        submitJob(it->execFile,it->ipFile);
        it++;
    }
    nodeToJob.erase(failnodeid);
    sentNodes.erase(failnodeid);
    return;
}

void Node::receive_result(string nodeid,string jobid,string opfile) {
    opfile=filerename(opfile);
    set<pair<string,int> >::iterator it=inputMapping[jobid].begin();
    while(it!=inputMapping[jobid].end() && it->first!=nodeid) it++;
    if(it==inputMapping[jobid].end()) return;
    result[jobid].insert(pair<int,string>(it->second,opfile));
    while(result[jobid].size()==inputMapping[jobid].size()) {
        Application app;
        string of=app.merge(result[jobid]);
        result.erase(jobid);
        inputMapping.erase(jobid);
        of=filerename(of);
        if(parent.find(jobid)!=parent.end()) {
            string j1=parent[jobid].first;
            result[j1].insert(pair<int,string>(parent[jobid].second,of));
            parent.erase(jobid);
            jobid=j1;
        } else {
            cout<< "Output of "<<md5_original[jobid]<<" stored in "<< of<< endl;
            md5_original.erase(jobid);
            break;
        }
    }
    
    if(nodeToJob[nodeid].size()==1) {
        nodeToJob.erase(nodeid);
        sentNodes.erase(nodeid); 
    } else {
        vector<Job>::iterator it=nodeToJob[nodeid].begin();
        while(it->jobId!=jobid) it++;
        nodeToJob[nodeid].erase(it);
    }
    return; 
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
            strcpy(buffer1,string("success").c_str());
            if(buffer[0] == InputSend + '0' and isLast == 1){
                globalQ.push_back(inputJobMapping[fileName]);
            }
        }
        else if(buffer[0] == Mapping + '0'){
            string str(buffer);
            int idx = str.find("::");
            int idx1 = str.find(":",idx+2);
            int idx2 = str.find(":",idx1+1);
            int idx3 = str.find(":",idx2+1);
            struct Job job;
            job.execFile = str.substr(idx+2, idx1-idx-2);
            job.ipFile = str.substr(idx1+1,idx2-idx1-1);
            job.jobId = str.substr(idx2+1, idx3-idx2-1);
            job.ownerId = str.substr(idx3+1);
            inputJobMapping[job.ipFile] = job;
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
        string ret = sendMessage(ip, port, buffer);

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

void Node::mapFilenametoJobId(string ip, string port, string execFileName, string ipFileName, string jobId, string ownerId)
{
    char* message = (char*)malloc(MAX*sizeof(char));
    sprintf(message, "%d::%s:%s:%s:%s", Mapping, execFileName.c_str(), ipFileName.c_str(), jobId.c_str(), ownerId.c_str());
    string ret = sendMessage(ip, port, message);
    free(message);
}

string Node::getIp(){
	return this->ip;
}

string Node::getPort(){
	return this->port;
}