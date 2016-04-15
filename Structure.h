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
#include <signal.h>
#include <future>
#include <chrono>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <fstream>
#include "md5.h"

using namespace std;

#define CheckAlive 1
#define ReplyAlive 2
#define JobAssign 3
#define Result 4
#define IAmUp 5
#define JobSend 6
#define InputSend 7
#define Mapping 8
#define HeartBeatTime 10
#define TIMEOUT 1
#define MUTEX 25

#define MAX 256
#define MAX1 10

inline string filerename(string from) {
    MD5 md5;
    char buf[256];
    strcpy(buf,from.c_str());
    string exf(md5.digestFile(buf));
    rename(from.c_str(),exf.c_str());
    return exf;
}

void down(int sem_id)
{
	struct sembuf sop;
	sop.sem_num = 0;
	sop.sem_op = -1;
	sop.sem_flg = 0;
	semop(sem_id,&sop,1);
}

void up(int sem_id)
{
	struct sembuf sop;
	sop.sem_num = 0;
	sop.sem_op = 1;
	sop.sem_flg = 0;
	semop(sem_id,&sop,1);
}

struct Job{
	string execFile, ipFile;
	string jobId,ownerId; //ownerId is ID of owner
};

string ips[10] = {"127.0.0.1",
				 "127.0.0.1",
				 "127.0.0.1",
				 "127.0.0.1",
				 "127.0.0.1",
				 "127.0.0.1",
				 "127.0.0.1",
				 "127.0.0.1",
				 "127.0.0.1",
				 "127.0.0.1"};


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