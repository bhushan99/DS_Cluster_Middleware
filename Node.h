#ifndef NODE_H
#define NODE_H
#include "Structure.h"

class Node{
public:
	Node(string ip, string port);
	void startUp();
	void submitJob(string execFileName, string ipFileName);
	void heartBeat();
	string getIp();
	string getPort();

private:
	string ip,port;
	queue<Job> localQ,globalQ;  // define struct
	set<int> sentNodes; // have to send heartbeat message to this nodes.
	map<int, set<int> > nodeToJob; // mapping for nodeid to set of jobid
	map<int, pair<int, pair<int,int> > > inputMapping; // mapping of jobId to pair of nodeId and input (pair of index)
};

#endif
