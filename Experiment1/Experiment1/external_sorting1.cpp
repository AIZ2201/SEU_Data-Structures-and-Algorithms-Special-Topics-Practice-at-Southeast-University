#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <string>
using namespace std;

void data_generation(int amount, string fileName)
{
	srand(time(NULL));

	ofstream fileOut(fileName);

	for (int i = 0; i < amount; i++)
	{
		int tempInt = rand() % 1001;
		fileOut << tempInt << " ";
		if (i % 10 == 9)
			fileOut << endl;
	}

	fileOut.close();
}

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

void segmentInitial(string inFileName, int segmentSize, queue<vector<int>>& disk, string outFileName) // outFileName��Ϊ��ȷ��˳���Ƿ���ȷ��ʼ��
{
	ifstream fileIn(inFileName);
	ofstream fileOut(outFileName);

	vector<int> segment;
	int num;

	while (fileIn >> num)
	{
		segment.push_back(num);

		if (segment.size() == segmentSize)
		{
			insertionSort(segment);

			for (int i = 0; i < segment.size(); i++)
			{
				fileOut << segment[i] << " ";
				if (i % 10 == 9)
					fileOut << endl;
			}

			disk.push(segment);

			segment.clear(); // ���segment����Ӧ����ڴ����
		}
	}

	if (!segment.empty())
	{
		insertionSort(segment);

		for (int i = 0; i < segment.size(); i++)
		{
			fileOut << segment[i] << " ";
			if (i % 10 == 9)
				fileOut << endl;
		}

		disk.push(segment);

		segment.clear(); // ���segment����Ӧ����ڴ����
	}

	fileIn.close();
	fileOut.close();
}

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

	while (!tempDisk1.empty() || !tempDisk2.empty() || !inputBuffer1.empty() || !inputBuffer2.empty())
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

void external_sort(string inFileName, int inputBufferSize, queue<vector<int>>& disk, string outFileName, int outputBufferSize)
{
	segmentInitial(inFileName, inputBufferSize, disk, outFileName);

	while (disk.size() > 1)
	{
		vector<int> tempDisk1 = disk.front();
		disk.pop();
		vector<int> tempDisk2 = disk.front();
		disk.pop();

		vector<int> outputDisk = EXT_merge_sort(tempDisk1, tempDisk2, inputBufferSize, outputBufferSize);

		disk.push(outputDisk);
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

int main()
{
	int N = 500; // ��500��������������

	string inFileName = "input.txt"; // ��txt�ļ�ģ�����
	queue<vector<int>> disk; // ��vector����ģ�����
	string outFileName = "output.txt"; // ��txt�ļ�ģ�����

	// ��Ҫ���ļ�ģ������еĴ����ˣ�������vector������ģ����������еĴ���

	int outputBufferSize = 20;
	int inputBufferSize = 10;

	data_generation(N, inFileName);

	external_sort(inFileName, inputBufferSize, disk, outFileName, outputBufferSize);
}