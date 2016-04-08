#include "Application.h"

Application::Application(){ }

vector<Job> Application::split(Job job, int n){

}

int Application::merge(vector<int> result){
	int sz = result.size();
	int finalResult = 0;
	for(int i=0; i<sz; i++){
		finalResult += result[i];
	}
	return finalResult;
}