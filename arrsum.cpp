#include <bits/stdc++.h>
#include <unistd.h>

using namespace std;

int main(int argc, char const *argv[])
{
	ifstream is;
	is.open(argv[1]);
	int ans=0,x;
	while(is>>x) {
		ans+=x;
		cout<<"Reading Input "<<x<<endl;
		sleep(1);
	}
	is.close();
	ofstream os;
	os.open(argv[2]);
	os<<ans<<endl;
	os.close();
	cout<<"Done!\n";
	return 0;
}