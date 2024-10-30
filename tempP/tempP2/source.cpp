#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

void insertionSort(vector<int>& array)
{
	for (int i = 1; i < array.size(); i++)
	{
		int temp = array[i];
		int j = i - 1;

		while (j >= 0 && array[j] > temp)
		{
			array[j + 1] = array[j];
			j -= 1;
		}
		array[j + 1] = temp;
	}
}

int main()
{
	string inFileName = "input.txt";
	string outFileName = "val.txt";
	vector<int> number;

	ifstream fileIn(inFileName);
	ofstream fileOut(outFileName);

	int num;

	while (fileIn >> num)
	{
		number.push_back(num);
	}

	fileIn.close();

	insertionSort(number);

	int i = 0;

	for (int temp : number)
	{
		fileOut << temp << " ";
		if (i % 10 == 9)
			fileOut << endl;
		i++;
	}

	fileOut.close();
}