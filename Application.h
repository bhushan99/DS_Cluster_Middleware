#ifndef APPLICATION_H
#define APPLICATION_H

#include "Structure.h"

class Application{
public:
	Application();
	vector<Job> split(Job job, int n);
	int merge(vector<int> result);

};

#endif