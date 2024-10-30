#include <iostream>
#include <vector>
#include <string>
#include <fstream>
using namespace std;

vector<int> memoryInput(vector<int>& disk, int inputBufferSize)
{
	int temp;
	vector<int> inputBuffer;

	while (!disk.empty() && inputBuffer.size() < inputBufferSize)
	{
		temp = disk.front();
		inputBuffer.push_back(temp);
		disk.erase(disk.begin());
	}

	return inputBuffer;
}

void memoryOutput(vector<int>& tempDisk, vector<int>& outputBuffer)
{
	tempDisk.insert(tempDisk.end(), outputBuffer.begin(), outputBuffer.end());

	outputBuffer.clear();
}

vector<int> EXT_merge_sort(vector<int>& tempDisk1, vector<int>& tempDisk2, int inputBufferSize, int outputBufferSize)
{
	vector<int> outputDisk;
	vector<int> inputBuffer1;
	vector<int> inputBuffer2;
	vector<int> outputBuffer;

	while (!tempDisk1.empty() || !tempDisk2.empty())
	{
		while (outputBuffer.size() < outputBufferSize)
		{
			if (!tempDisk1.empty() && inputBuffer1.empty())
				inputBuffer1 = memoryInput(tempDisk1, inputBufferSize);
			if (!tempDisk2.empty() && inputBuffer2.empty())
				inputBuffer2 = memoryInput(tempDisk2, inputBufferSize);

			if (inputBuffer1.empty())
			{
				if (inputBuffer2.empty())
					break;

				outputBuffer.push_back(inputBuffer2.front());
				inputBuffer2.erase(inputBuffer2.begin());
			}
			else if (inputBuffer2.empty())
			{
				outputBuffer.push_back(inputBuffer1.front());
				inputBuffer1.erase(inputBuffer1.begin());
			}
			else
			{
				int temp1 = inputBuffer1.front();
				int temp2 = inputBuffer2.front();

				if (temp1 <= temp2)
				{
					outputBuffer.push_back(temp1);
					inputBuffer1.erase(inputBuffer1.begin());
				}
				else
				{
					outputBuffer.push_back(temp2);
					inputBuffer2.erase(inputBuffer2.begin());
				}
			}
		}

		memoryOutput(outputDisk, outputBuffer);
	}

	return outputDisk;
}

int main()
{
	int inputBufferSize = 10, outputBufferSize = 20;
	string outFileName = "output.txt";

	int a[10] = {1,2,3,4,5,6,7,8,29,30};
	int b[20] = {9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28};

	vector<int> temp;
	vector<vector<int>> disk;

	for (int i = 0; i < 10; i++)
	{
		temp.push_back(a[i]);
	}
	disk.push_back(temp);
	temp.clear();
	for (int i = 0; i < 20; i++)
	{
		temp.push_back(b[i]);
	}
	disk.push_back(temp);
	temp.clear();

	while (disk.size() > 1)
	{
		vector<int> tempDisk1 = disk.front();
		disk.erase(disk.begin());
		vector<int> tempDisk2 = disk.front();
		disk.erase(disk.begin());

		vector<int> outputDisk = EXT_merge_sort(tempDisk1, tempDisk2, inputBufferSize, outputBufferSize);

		disk.push_back(outputDisk);
	}

	vector<int> result = disk.front();

	ofstream fileOut(outFileName);

	int i = 0;

	for (int temp : result)
	{
		fileOut << temp << " ";
		if (i % 10 == 9)
			fileOut << endl;
		i++;
	}

	fileOut.close();
}