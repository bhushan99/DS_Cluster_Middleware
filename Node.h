#ifndef NODE_H
#define NODE_H
#include "Structure.h"
#define MAX 256
#define BACKLOG 10

class Node{
public:
	Node(string ip, string port);
	void startUp();
	void submitJob(string execFileName, string ipFileName);
	void heartBeat();
	string getIp();
	string getPort();
	string sendMessage(string ip, string port, string msg);
	void receiveMessage();

private:
	string ip,port;
	queue<Job> localQ,globalQ;
	set<int> sentNodes; // have to send heartbeat message to this nodes.
	map<int, set<int> > nodeToJob; // mapping for nodeid to set of jobid
	map<int, pair<int, pair<int,int> > > inputMapping; // mapping of jobId to pair of nodeId and input (pair of index)
};

#endif
