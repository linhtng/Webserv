#include <fstream>

int main()
{
	std::ofstream ofs("number.txt");

	for (int i = 0; i < 1000000; i++)
		ofs << i;

	ofs.close();
	return 0;
}