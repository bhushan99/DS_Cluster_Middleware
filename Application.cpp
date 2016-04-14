#include "Application.h"

Application::Application(){ }

vector<Job> Application::split(Job job, int n){
	string in=job.ipFile, ex=job.execFile;
	vector<Job> ans;
	ifstream is;
	is.open(in.c_str());
	int x;
	int ct=0;
	while(!is.eof()) {is>>x;ct++;}
	is.close();
	is.open(in.c_str());
	int p=(ct+n-1)/n;
	ofstream os;
	for(int i=1;i<=n;i++) {
		Job j;
		j.execFile=ex;
		stringstream ss;
		ss << i;
		j.ipFile=in+string("_part")+ss.str();
		j.ownerId=job.ownerId;
		os.open(j.ipFile.c_str());
		int k=0;
		while(k<p && ct) {
			is>>x;
			os<<x<<endl;
			k++; ct--;
		}
		os.close();
    	j.jobId = job.jobId;
		ans.push_back(j);
		if(!ct) break;
	}
	is.close();
	return ans;
}

int Application::merge(vector<int> result){
	int sz = result.size();
	int finalResult = 0;
	for(int i=0; i<sz; i++){
		finalResult += result[i];
	}
	return finalResult;
}