#include "Node.h"

Node::Node(string ip, string port):ip(ip),port(port) { }

void Node::startUp(){
	
	for(int i=0; i<2; i++){
		sendMessage(ips[i],ports[i],"1:");
	}
}

void Node::submitJob(string execFileName, string ipFileName){
	
}

void Node::heartBeat(){

}

void Node::sendMessage(string ip, string port, string msg){
	int ps_id;
	struct sockaddr_in ps_addr;

	if((ps_id=socket(AF_INET,SOCK_STREAM,0))==-1){
		perror("Socket() for Communication");
		return;
	}
	ps_addr.sin_family=AF_INET;
	ps_addr.sin_addr.s_addr=inet_addr(ip.c_str());
	ps_addr.sin_port=htons(atoi(port.c_str()));
	int ps_len=sizeof(ps_addr);
	pid_t pid = fork();
	if(pid == 0){
		// child process
		cout << "sending IAmUp message to " << ip << endl;
		if(connect(ps_id,(struct sockaddr *)&ps_addr,ps_len)==-1){
			perror("connect()");
			cout<<"Cannot connect to given ip: " << ip << endl ;
			return;
		}
		write(ps_id,msg.c_str(),MAX);
	}
	else{
		sleep(1);
		kill(pid,SIGKILL);
	}
	return;
}



void Node::receiveMessage(){
	int tr_id, cli_id;
	struct sockaddr_in addrport, cli_addr;

	if((cli_id=socket(AF_INET,SOCK_STREAM,0))==-1){
		perror("Socket() for Peer CLient:");
		exit(1);
	}

	addrport.sin_family=AF_INET;
	addrport.sin_addr.s_addr=INADDR_ANY;
	addrport.sin_port=htons(atoi(port.c_str()));
	if(bind(cli_id,(struct sockaddr *)&addrport,sizeof(addrport))==-1)
		perror("bind()");
	cout << "check" << endl;
	while(1){
		cout << "check1" << endl;
		if(listen(cli_id,BACKLOG)==-1)
			perror("listen()");
		int clilen=sizeof(cli_addr);
		tr_id=accept(cli_id,(struct sockaddr *)&cli_addr,(socklen_t *)&clilen);
		if(tr_id==-1){
			perror("accept()");
			continue;
		}
		if(fork()==0)
			break;
		cout << "check2" << endl;
	}

	int ipAddr=cli_addr.sin_addr.s_addr;
	char str[INET_ADDRSTRLEN];
	inet_ntop( AF_INET, &ipAddr, str, INET_ADDRSTRLEN );
	char buf[MAX],*buffer=(char*)malloc(MAX*sizeof(char));
	read(tr_id,buf,MAX);
	cout << "check3" << endl;
	cout << buf << endl;

	// send message to client that it is also alive

	// int ps_id;
	// struct sockaddr_in ps_addr;

	// if((ps_id=socket(AF_INET,SOCK_STREAM,0))==-1){
	// 	perror("Socket() for Communication");
	// 	return;
	// }
	// ps_addr.sin_family=AF_INET;
	// ps_addr.sin_addr.s_addr=inet_addr(ip.c_str());
	// ps_addr.sin_port=htons(atoi(port.c_str()));
	// int ps_len=sizeof(ps_addr);
	// pid_t pid = fork();
	// if(pid == 0){
	// 	// child process
	// 	cout << "sending IAmUp message to " << ip << endl;
	// 	if(connect(ps_id,(struct sockaddr *)&ps_addr,ps_len)==-1){
	// 		perror("connect()");
	// 		cout<<"Cannot connect to given ip: " << ip << endl ;
	// 		return;
	// 	}
	// 	write(ps_id,msg.c_str(),MAX);
	// }
	// else{
	// 	sleep(1);
	// 	kill(pid,SIGKILL);
	// }
}

string Node::getIp(){
	return this->ip;
}

string Node::getPort(){
	return this->port;
}