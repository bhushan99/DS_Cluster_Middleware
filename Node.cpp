#include "Node.h"
#include <fstream>
Node::Node(string ip, string port):ip(ip),port(port) { ID=ip+string("<")+port; }

void Node::startUp(){
	thread listenMessage (&Node::receiveMessage,this);
    listenMessage.detach();

    thread listenFile(&Node::receiveExecFile,this);
    listenFile.detach();

    thread executeJob1(&Node::executeJob, this);
    executeJob1.detach();

    // thread submitJobT(&Node::submitJobThread, this);
    // submitJobT.detach();
    	
	for(int i=0; i<10; i++){
		
		if(port != ports[i]){
			string res = sendMessage(ips[i],ports[i],to_string(IAmUp)+"::"+ip+"<"+port);
			if(res == "error")
				continue;
			else if(res[0] == '2'){
                int idx = res.find("::");
				sentNodes.insert(res.substr(idx+2));
			}
		}
	}
	cout << "Total number of nodes in sentNodes : " <<  sentNodes.size() << endl;

    // thread heartBeatMessage (&Node::heartBeat,this);
    // heartBeatMessage.detach();
	
}

void Node::executeJob(){
    while(1){
        sleep(5);
        cout<<getpid()<< " GQ : "<<globalQ.size()<<endl;
        if(globalQ.empty())
            continue;
        cout << "thread working fine " << globalQ.size() << endl;
        Job job = globalQ.front();
        globalQ.pop_front();
        cout << "execFile name is: " << job.execFile << endl;
        cout << "ipFile name is: " << job.ipFile << endl;
        cout << "jobId is: " << job.jobId << endl;
        cout << "ownerId is: " << job.ownerId << endl;
        cout << "Job is being executed" << endl;
        char *arglist[4];
        arglist[0] = (char *)malloc(256*sizeof(char)); strcpy(arglist[0],job.execFile.c_str());
        arglist[1] = (char *)malloc(256*sizeof(char)); strcpy(arglist[1],job.ipFile.c_str());
        arglist[2] = (char *)malloc(256*sizeof(char)); strcpy(arglist[2],(string("out_")+job.ipFile).c_str());
        arglist[3] = NULL;
        char buf[256];
        cout << "testing" << endl;
        if(job.execFile[0]=='/') sprintf(buf,"%s",job.execFile.c_str());
        else sprintf(buf,"%s/%s",get_current_dir_name(),job.execFile.c_str());
        int pid, status;
        if((pid = fork()) == 0){
            execvp(buf,arglist);
        }
        free(arglist[0]);
        free(arglist[1]);
        free(arglist[2]);
        waitpid(pid,&status,0);
        cout << "Job is finished" << endl;
        if(job.ownerId==ID) receive_result(ID,job.jobId,string("out_")+job.ipFile);
        else {
            pair<string, string> addr = split_(job.ownerId);
            cout<<job.ipFile.size()<<endl;
            sendExecFile(addr.first, addr.second, string("out_")+job.ipFile);
        }
    }
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

void Node::submitJob(string execFileName, string ipFileName){
    cout << execFileName << " " << ipFileName << endl;
    MD5 md5;
    char buf[256];
    strcpy(buf,execFileName.c_str());
    string exf(md5.digestFile(buf));
    strcpy(buf,ipFileName.c_str());
    string ipf(md5.digestFile(buf));
    Job j;
    j.execFile=execFileName;
    j.ipFile=ipFileName;
    j.jobId=exf+string("<")+ipf;
    j.ownerId=this->ID;
    md5_original[j.jobId]=execFileName+string("<")+ipFileName;
    localQ.push_back(j);
    load.erase(load.begin(),load.end());
    // SEND load query to all nodes, receive reply, fill load
    for(int i=0; i<MACHINES; i++){
        if(ID == ips[i]+"<"+ports[i])
            continue;
        string out = sendMessage(ips[i],ports[i],to_string(Query)+"::"+ip+"<"+port);
        if(out == "timeout" or out == "disconnect")
            continue;
        int temp = stoi(out);
        load.push_back(make_pair(ips[i]+"<"+ports[i],temp));
    }
    // getDestNodes(load);
    // cout << "I am here" << endl;
    Application app;
    vector<Job> vj= app.split(j,load.size()+1);
    // cout << "size of vector job is " << vj.size() << endl;
    for(int i=0;i<vj.size()-1;i++) //send files to nodes in load[i]
    {
        string sentip = load[i].first.substr(0,load[i].first.find("<"));
        string sentport = load[i].first.substr(load[i].first.find("<")+1);
        mapFilenametoJobId(sentip,sentport,vj[i].execFile,vj[i].ipFile,vj[i].jobId,vj[i].ownerId);
        sendExecFile(sentip, sentport, vj[i].ipFile);
        sendExecFile(sentip, sentport, vj[i].execFile);
        sentNodes.insert(load[i].first);
        nodeToJob[load[i].first].push_back(vj[i]);
        inputMapping[j.jobId].insert(pair<string,int>(load[i].first,i+1));
    }
    globalQ.push_back(vj[vj.size()-1]); //keep self part
    cout << getpid() <<"size of globalQ is " << globalQ.size() << endl;
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
                nodeFail(curNode);
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
        return "disconnect";
    }
    bzero(buffer,256);
    strcpy(buffer,msg.c_str());
    cout << "Sending message " << msg << " to " << ip + "<" + port << endl;
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
    cout << "Got message " << buffer << " from " << ip + "<" + port << endl;
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
        string newjid=exf+string("<")+ipf;
        j.jobId=newjid;
        Application app;
        vector<Job> vj= app.split(j,2);
        for(int i=0;i<vj.size()-1;i++) //send files to nodes in newnodeid
        {
            string sentip = newnodeid.substr(0,newnodeid.find("<"));
            string sentport = newnodeid.substr(newnodeid.find("<")+1);
            mapFilenametoJobId(sentip,sentport,vj[i].execFile,vj[i].ipFile,vj[i].jobId,vj[i].ownerId);
            sendExecFile(sentip, sentport, vj[i].ipFile);
            sendExecFile(sentip, sentport, vj[i].execFile);
                
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
        string newjid=exf+string("<")+ipf;
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
    // char buffer[256];
    char *buffer = (char*) malloc(MAX*sizeof(char));
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
    cout << "Ready to receive any message" << endl;
    while(1){
	    listen(sockfd,5);
	    clilen = sizeof(cli_addr);
	    
	    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	    if (newsockfd < 0) 
	        perror("accept()");
	    // bzero(buffer,256);
	    memset(buffer, 0, MAX);
        bzero(buffer1,256);
        int n = read(newsockfd,buffer,MAX);
	    if (n < 0) perror("ERROR reading from socket");
	    if(buffer[0] == IAmUp+'0'){
            string str(buffer);
            int idx = str.find("::");
	    	sentNodes.insert(str.substr(idx+2));   
	    	sprintf(buffer1,"%d::",ReplyAlive);
            strcat(buffer1,ip.c_str());
            strcat(buffer1,("<"+port).c_str());
	    }
        else if(buffer[0] == Mapping+'0'){
            string str(buffer);
            int idx = str.find("::");
            int idx1 = str.find(":",idx+2);
            int idx2 = str.find(":",idx1+1);
            int idx3 = str.find(":",idx2+1);
            int idx4 = str.find(":",idx3+1);
            string execName, ipName, jobId, ownerId;
            execName = str.substr(idx+2,idx1-idx-2);
            ipName = str.substr(idx1+1,idx2-idx1-1);
            jobId = str.substr(idx2+1,idx3-idx2-1);
            ownerId = str.substr(idx3+1,idx4-idx3-1);
            Job job;
            job.execFile = execName;
            job.ipFile = ipName;
            job.jobId = jobId;
            job.ownerId = ownerId;
            inputJobMapping[execName] = job;
            strcat(buffer1,string("success").c_str());
            cout << "inputJobMapping size: " << inputJobMapping.size() << endl;
            // cout << execName << " " << job << endl;
        }
        else if(buffer[0] == Query+'0'){
            int te = globalQ.size();
            sprintf(buffer1,"%d",te);
        }
	    cout << "size of set is " << sentNodes.size() << endl;
	    n = write(newsockfd,buffer1,256);
	    if (n < 0) perror("ERROR writing to socket");
	
    }
    close(newsockfd);
    close(sockfd);
}

void Node::receiveExecFile(){
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char *buffer = (char*) malloc(MAX*sizeof(char));
    char *buffer1 = (char*) malloc(MAX1*sizeof(char));
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        perror("Server Socket ");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi((port).c_str())+1000;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        perror("bind()");
    cout << "Ready to receive any File" << endl;
    while(1){
        listen(sockfd,5);
        clilen = sizeof(cli_addr);
        
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) 
            perror("accept()");
        // bzero(buffer,256);
        memset(buffer, 0, MAX);
        memset(buffer1, 0, MAX1);
        string fileName = "testing1";
        FILE *fp2;
        bool change = true;
        int noOfColons = 1;
        while(1){
            n = read(newsockfd,buffer,MAX);
            /**/
            string str = "";
            int i  = 0;
            for(i = 0; i<n and change == true; i++){
                if(noOfColons == 0){
                    change = false;
                    fp2 = fopen(str.substr(0,str.size()-1).c_str(),"wb");
                    fileName = str.substr(0,str.size()-1);
                    break;
                }
                str += buffer[i];
                if(buffer[i] == ':'){
                    noOfColons --;
                }
                
            }
            /**/
            if(n <= 0)
                break;

            fwrite(buffer+i,sizeof(char), n-i, fp2);
            free(buffer);
            buffer = (char*) malloc(MAX*sizeof(char));
            memset(buffer, 0, MAX);
        }
        fclose(fp2); 
        
        map<string,Job>::iterator it;
        for(it = inputJobMapping.begin(); it != inputJobMapping.end(); it++){
            cout << it->first << endl;
        }
        if(inputJobMapping.find(fileName) != inputJobMapping.end()){
            globalQ.push_back(inputJobMapping[fileName]);
            cout << "Job iserted in globalQ " << endl;
        }
        else{
            cout << "Mapping is not found" << endl;
        }

    }
}



void Node::sendExecFile(string ip, string port, string fileName)
{
    int fis_id,ps_id;
    struct sockaddr_in ps_addr,fis_addr;
    
    if((ps_id=socket(AF_INET,SOCK_STREAM,0))==-1){
        perror("Socket() for Peer Server");
        return;
    }
   
    ps_addr.sin_family=AF_INET;
    ps_addr.sin_addr.s_addr=inet_addr(ip.c_str());
    ps_addr.sin_port=htons(atoi(port.c_str())+1000);
    int ps_len=sizeof(ps_addr);

    if(connect(ps_id,(struct sockaddr *)&ps_addr,ps_len)==-1){
        perror("connect()");
        cout<<"Cannot connect to Peer Server\n";
        return;
    }
    
    FILE *fp=fopen(fileName.c_str(),"rb");
    fseek(fp,0,SEEK_END);
    int f_sz = ftell(fp);
    rewind(fp);
    int size = 0, nbytes = min(f_sz, MAX-1);
   
    char *buffer = (char*)malloc(MAX*sizeof(char)), *buffer1 = (char*)malloc(MAX1*sizeof(char));
    memset(buffer, 0, MAX);
    memset(buffer1, 0, MAX1);
    strcpy(buffer,(fileName+string("test")).c_str());
    int sz = fileName.size() + 4;
    // for(int i=0;i<10;i++)
        // buffer[i] = ':';
    buffer[sz++] = ':';
    write(ps_id,buffer,sz);
    free(buffer);
    buffer = (char*)malloc(MAX*sizeof(char));
    memset(buffer,0,MAX);
    while((size = fread(buffer,sizeof(char), nbytes, fp)) > 0)
    {
        
        write(ps_id, buffer, size);
        free(buffer);
        buffer = (char*)malloc(MAX*sizeof(char));
        memset(buffer,0,MAX);

        f_sz -= size;
        nbytes = min(f_sz, MAX-1);
    }
    fclose(fp);
    shutdown(ps_id,2);
    cout << "exec file transfer successful" << endl;
    
}


void Node::mapFilenametoJobId(string ip, string port, string execFileName, string ipFileName, string jobId, string ownerId)
{
    char* message = (char*)malloc(MAX*sizeof(char));
    sprintf(message, "%d::%s:%s:%s:%s:", Mapping, (execFileName+string("test")).c_str(), ipFileName.c_str(), jobId.c_str(), ownerId.c_str());
    string ret = sendMessage(ip, port, message);
    free(message);
    return ;
}

void Node::submitJobThread() {
    string a,b;
    while(1) {
        cout<< "Enter Executable File: ";
        cin>>a;
        cout<< "Enter Input File: ";
        cin>>b;
        submitJob(a,b);
    }
}

string Node::getIp(){
	return this->ip;
}

string Node::getPort(){
	return this->port;
}