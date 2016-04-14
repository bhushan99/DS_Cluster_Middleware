#ifndef NODE_H
#define NODE_H
#include "Structure.h"
#include "Application.cpp"
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
	void mapFilenametoJobId(string ip, string port, string execFileName, string ipFileName, string jobId, string ownerId);
private:
	string ip,port,ID; // ID= ip+":"+port, jobID= exFile+":"+ipFile
	deque<Job> localQ,globalQ;
	set<string> sentNodes; // have to send heartbeat message to this nodes.
	map<string, set<Job> > nodeToJob; // mapping for nodeid to set of job
	map<string, FILE *> filePointer;
	map<string, FILE *> inputPointer;
	map<string, set<pair<string, int> > > inputMapping; // mapping of jobId to pair of nodeId and index
	map<string,string> md5_original; //md5 Job to original file names in job
	vector<pair <string,int> > load; // info of #jobs in waiting Q per nodeID
	map<string, Job> inputJobMapping; // mapping of input file to Job
	map<string,string> parent; // point to parent job on same node
};

#endif
