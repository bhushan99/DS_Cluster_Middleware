#include "Node.cpp"
#include "Application.cpp"

int main(){
	Node node = Node("10.3.100.207","1234");
	cout << node.getIp() << " " << node.getPort() << endl;
	Application app = Application();
	vector<int> vec;
	for(int i=0; i<10; i++){
		vec.push_back(i+1);
	}
	cout << app.merge(vec) << endl;
	return 0;
}