#include "Node.h"

Node::Node(string ip, string port):ip(ip),port(port) { }

void Node::startUp(){

}

void Node::submitJob(string execFileName, string ipFileName){
	
}

void Node::heartBeat(){

}

string Node::getIp(){
	return this->ip;
}

string Node::getPort(){
	return this->port;
}