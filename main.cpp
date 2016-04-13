#include "Node.cpp"
#include "Application.cpp"

int main(int argc, char *argv[]){
	Node node = Node(argv[1],argv[2]);
	node.startUp();
	return 0;
}