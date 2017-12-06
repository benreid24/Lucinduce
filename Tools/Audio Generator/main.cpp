#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
using namespace std;

int main()
{
	srand(time(NULL));

	int s = 0;
	cout << "Generate how many samples? ";
	cin >> s;

	ofstream file("data.txt");
	file << "{";
	for (int i = 0; i<s; ++i)
	{
		file << rand()%8900+100;
		if (i!=s-1)
			file << ",\n";
	}
	file << "};";
	file.close();

	return 0;
}
