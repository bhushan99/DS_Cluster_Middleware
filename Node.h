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
	string sendFile(string ip, string port, string fileName, int type);
	void receiveFile();

private:
	string ip,port,ID; // ID= ip+":"+port, jobID= exFile+":"+ipFile
	queue<Job> localQ,globalQ;
	set<string> sentNodes; // have to send heartbeat message to this nodes.
	map<string, set<string> > nodeToJob; // mapping for nodeid to set of jobid
	map<string, pair<string, int > > inputMapping; // mapping of jobId to pair of nodeId and index
	map<string,string> md5_original; //md5 Job to original file names in job
	vector<pair <string,int> > load; // info of #jobs in waiting Q per nodeID
};

#endif
