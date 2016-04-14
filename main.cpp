#include "Node.cpp"
#include "Application.cpp"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#define PARENT 1
#define CHILD 2


int main(int argc, char *argv[]){
	Node node = Node(argv[1],argv[2]);
	node.startUp();

	char execFile[100], inputFile[100];
	string choice, ipFileName, execFileName;
	int pid, parentSemId, childSemId, mutex;
	int process = 0;
	struct sembuf sop;

	// parentSemId = semget((key_t)PARENT, 1, IPC_CREAT | 0666);
	// childSemId = semget((key_t)CHILD, 1, IPC_CREAT | 0666);
	mutex = semget((key_t)MUTEX, 1, IPC_CREAT | 0666);
	semctl(mutex, 0, SETVAL, 1);

	while(1)
	{
		down(mutex);
		cout << "Do you want to submit job? (y/n) : ";
		cin >> choice;

		if(choice[0] == 'y')
		{
			cout << "Enter executable file: ";
			cin >> execFile;
			cout << "Enter input file: ";
			cin >> inputFile;

			char fullPathEx[PATH_MAX], fullPathInp[PATH_MAX];
			realpath(execFile, fullPathEx);
			realpath(inputFile, fullPathInp);
			
			string execFileName (fullPathEx);
			string ipFileName (fullPathInp);

			pid = fork();
			if(pid == 0)
			{
				node.submitJob(execFileName, ipFileName);
				exit(0);
			}
			else
			{
				wait(&pid);
			}
		}
		else if(choice[0] == 'n')
		{
			up(mutex);
			continue;
		}
		else
		{
			cout << "Enter y or n!!\n";
		}
		up(mutex);
	}
	return 0;
}