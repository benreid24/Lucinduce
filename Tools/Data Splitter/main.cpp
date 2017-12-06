#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int main()
{
	string filename, line;
	ifstream in;
	ofstream out1, out2;

	cout << "Filename: ";
	getline(cin,filename);

	in.open(filename.c_str());
	out1.open(string(filename+"_avg.txt").c_str());
	out2.open(string(filename+"_max.txt").c_str());
	while (in.good())
	{
		getline(in,line);
		out1 << line << endl;
		getline(in,line);
		out2 << line << endl;
	}
	in.close();
	out1.close();
	out2.close();

	return 0;
}
