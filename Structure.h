#ifndef STRUCTURE_H
#define STRUCTURE_H

#include <bits/stdc++.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>

using namespace std;

struct Job{
	string execFile, ipFile;
	int jobId;
};

string ips[10] = {"10.0.0.1",
				 "10.0.0.2",
				 "10.0.0.3",
				 "10.0.0.4",
				 "10.0.0.5",
				 "10.0.0.6",
				 "10.0.0.7",
				 "10.0.0.8",
				 "10.0.0.9",
				 "10.0.0.10"};

string ports[10] = {"12341",
				 "12342",
				 "12343",
				 "12344",
				 "12345",
				 "12346",
				 "12347",
				 "12348",
				 "12349",
				 "12350"};

#endif