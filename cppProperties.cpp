#include <bits/stdc++.h>
#include "cppheader.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
using namespace std;

#define debug(var) cerr<<#var<<" = "<<(var)<<endl;

enum e1{
	X=2,Y,Z=5
};

void func(int i=2,int j=1,char k){
	i=1;
}

int lval(const int& x) {
	return x+1;
}

void references() {
	int i,j=5,k=10;
	int &x=j;
	debug(x);
	int* p=&x;
	(*p)++;
	debug(j);
	const int& y=j+k;
}

int main() {
    char* arglist[4];
    arglist[0] = (char *)malloc(7*sizeof(char)); strcpy(arglist[0],"arrsum");
    arglist[1] = (char *)malloc(6*sizeof(char)); strcpy(arglist[1],"ip.in");
    arglist[2] = (char *)malloc(7*sizeof(char)); strcpy(arglist[2],"op.out");
    arglist[3] = NULL;
    char buf[256];
    sprintf(buf,"%s/arrsum",get_current_dir_name());
	execvp(buf,arglist);
	return 0;
}