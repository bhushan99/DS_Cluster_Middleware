#include "Node.cpp"
#include "Application.cpp"

int main(int argc, char *argv[]){
	Node node = Node(argv[1],argv[2]);
	cout << node.getIp() << " " << node.getPort() << endl;
	node.startUp();
	Application app = Application();
	vector<int> vec;
	for(int i=0; i<10; i++){
		vec.push_back(i+1);
	}
	cout << app.merge(vec) << endl;
	return 0;
}