#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <string>
#include <thread>
#include <mutex>
using namespace std;

thread inputThread1;
thread inputThread2;
thread outputThread1;
thread outputThread2;
mutex inputLock;
mutex outputLock;

int disk_action_amount_pre = 0;
int disk_action_amount_run = 0;
int disk_action_amount_sum = 0;

struct Compare {
	bool operator()(const vector<int>& a, const vector<int>& b) {
		return a.size() > b.size(); // 元素少的在前（先被merge）
	}
};

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

vector<int> memoryInput(vector<int>& disk, int inputBufferSize)
{
	disk_action_amount_sum++;

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

void memoryInputT(vector<int>& disk, int inputBufferSize, vector<int>& inputBuffer)
{
	inputLock.lock();

	disk_action_amount_sum++;

	int temp;

	while (!disk.empty() && inputBuffer.size() < inputBufferSize)
	{
		temp = disk.front();
		inputBuffer.push_back(temp);
		disk.erase(disk.begin());
	}

	inputLock.unlock();
}

void memoryOutput(vector<int>& tempDisk, vector<int>& outputBuffer)
{
	disk_action_amount_sum++;

	tempDisk.insert(tempDisk.end(), outputBuffer.begin(), outputBuffer.end());

	outputBuffer.clear();
}

void memoryOutputT(vector<int>& tempDisk, vector<int>& outputBuffer)
{
	outputLock.lock();

	disk_action_amount_sum++;

	tempDisk.insert(tempDisk.end(), outputBuffer.begin(), outputBuffer.end());

	outputBuffer.clear();

	outputLock.unlock();
}

void memoryOutputTT(priority_queue<vector<int>, vector<vector<int>>, Compare>& disk, vector<int>& tempDisk, vector<int>& outputBuffer)
{
	outputLock.lock();

	disk_action_amount_sum++;

	tempDisk.insert(tempDisk.end(), outputBuffer.begin(), outputBuffer.end());

	outputBuffer.clear();

	outputLock.unlock();

	disk.push(tempDisk);
	tempDisk.clear();
}

void segmentInitial(string inFileName, int inputBufferSize, int outputBufferSize, priority_queue<vector<int>, vector<vector<int>>, Compare>& disk, string outFileName) {
	ifstream fileIn(inFileName);
	vector<int> segment;

	// 从文件中读取数据到 segment
	int num;
	while (fileIn >> num) {
		segment.push_back(num);
	}
	fileIn.close();

	// 败者树的初始化
	const int N = 16; // 败者树的大小
	int loserTree[N] = { 0 }; // 败者树
	int winnerTree[N / 2] = { 0 }; // 胜者索引
	bool loserTreeStatus[N] = { };
	for (int i = 0; i < N; i++) // 非锁定状态
		loserTreeStatus[i] = true;

	vector<int> outputBuffer1, outputBuffer2, run; // 输出缓冲区和生成的顺串
	vector<int> inputBuffer1, inputBuffer2; // 输入缓冲区

	int currentInputBuffer = 1; // 1 表示使用 inputBuffer1，2 表示使用 inputBuffer2
	int currentOutputBuffer = 1; // 1 表示使用 outputBuffer1，2 表示使用 outputBuffer2

	inputBuffer1 = memoryInput(segment, inputBufferSize);
	// inputBuffer2 = memoryInput(segment, inputBufferSize);
	inputThread2 = thread(memoryInputT, ref(segment), inputBufferSize, ref(inputBuffer2));

	for (int i = 8; i < 16; i++) {
		loserTree[i] = inputBuffer1.front();
		inputBuffer1.erase(inputBuffer1.begin());
	}

	// 败者树初始化
	for (int i = 4; i < 8; i++) {
		if (loserTree[2 * i] > loserTree[2 * i + 1]) {
			loserTree[i] = 2 * i;
			winnerTree[i] = 2 * i + 1;
		}
		else {
			loserTree[i] = 2 * i + 1;
			winnerTree[i] = 2 * i;
		}
	}

	for (int i = 2; i < 4; i++) {
		if (loserTree[winnerTree[2 * i]] > loserTree[winnerTree[2 * i + 1]]) {
			loserTree[i] = winnerTree[2 * i];
			winnerTree[i] = winnerTree[2 * i + 1];
		}
		else {
			loserTree[i] = winnerTree[2 * i + 1];
			winnerTree[i] = winnerTree[2 * i];
		}
	}

	if (loserTree[winnerTree[2]] > loserTree[winnerTree[3]]) {
		loserTree[1] = winnerTree[2];
		winnerTree[1] = winnerTree[3];
		loserTree[0] = winnerTree[3];
	}
	else {
		loserTree[1] = winnerTree[3];
		winnerTree[1] = winnerTree[2];
		loserTree[0] = winnerTree[2];
	}

	outputThread2 = thread(memoryOutputT, ref(run), ref(outputBuffer2));

	while (1) {
		if (segment.empty() && inputBuffer1.empty() && inputBuffer2.empty())
			break;

		// 胜者输出与新值输入
		int winner = loserTree[loserTree[0]];
		if (currentOutputBuffer == 1) {
			outputBuffer1.push_back(winner);
			if (outputBuffer1.size() == outputBufferSize) {
				outputThread2.join();
				outputThread1 = thread(memoryOutputT, ref(run), ref(outputBuffer1));
				currentOutputBuffer = 2; // 切换到 outputBuffer2
			}
		}
		else {
			outputBuffer2.push_back(winner);
			if (outputBuffer2.size() == outputBufferSize) {
				outputThread1.join();
				outputThread2 = thread(memoryOutputT, ref(run), ref(outputBuffer2));
				currentOutputBuffer = 1; // 切换到 outputBuffer1
			}
		}

		// 输入Buffer处理
		if (currentInputBuffer == 1) {
			loserTree[loserTree[0]] = inputBuffer1.front();
			inputBuffer1.erase(inputBuffer1.begin());

			if (inputBuffer1.empty() && !segment.empty()) {
				inputThread2.join();
				inputThread1 = thread(memoryInputT, ref(segment), inputBufferSize, ref(inputBuffer1));
				currentInputBuffer = 2; // 切换到 inputBuffer2
			}

			if (inputBuffer1.empty() && segment.empty())
			{
				currentInputBuffer = 2; // 切换到 inputBuffer2
			}
		}
		else {
			loserTree[loserTree[0]] = inputBuffer2.front();
			inputBuffer2.erase(inputBuffer2.begin());

			if (loserTree[loserTree[0]] == 17)
			{
				int a = 0;
			}

			if (inputBuffer2.empty() && !segment.empty()) {
				inputThread1.join();
				inputThread2 = thread(memoryInputT, ref(segment), inputBufferSize, ref(inputBuffer2));
				currentInputBuffer = 1; // 切换到 inputBuffer1
			}

			if (inputBuffer2.empty() && segment.empty())
			{
				currentInputBuffer = 1; // 切换到 inputBuffer1
			}
		}

		// 更新败者树
		if (loserTree[loserTree[0]] < winner) {
			loserTreeStatus[loserTree[0]] = false;

			for (int i = 8; i < 16; i++) {
				if (loserTreeStatus[i] == true)
					break;
				if (i == 15) {
					// 树全部锁定，截断顺串，解锁后重生成
					if (currentOutputBuffer == 1) {
						outputThread2.join();
						outputThread1 = thread(memoryOutputTT, ref(disk), ref(run), ref(outputBuffer1));
						currentOutputBuffer = 2; // 切换到 outputBuffer2
					}
					else {
						outputThread1.join();
						outputThread2 = thread(memoryOutputTT, ref(disk), ref(run), ref(outputBuffer2));
						currentOutputBuffer = 1; // 切换到 outputBuffer1
					}

					/*disk.push(run);
					run.clear();*/

					for (int j = 0; j < 16; j++) {
						loserTreeStatus[j] = true;
					}

					// 败者树初始化
					for (int i = 4; i < 8; i++) {
						if (loserTree[2 * i] > loserTree[2 * i + 1]) {
							loserTree[i] = 2 * i;
							winnerTree[i] = 2 * i + 1;
						}
						else {
							loserTree[i] = 2 * i + 1;
							winnerTree[i] = 2 * i;
						}
					}

					for (int i = 2; i < 4; i++) {
						if (loserTree[winnerTree[2 * i]] > loserTree[winnerTree[2 * i + 1]]) {
							loserTree[i] = winnerTree[2 * i];
							winnerTree[i] = winnerTree[2 * i + 1];
						}
						else {
							loserTree[i] = winnerTree[2 * i + 1];
							winnerTree[i] = winnerTree[2 * i];
						}
					}

					if (loserTree[winnerTree[2]] > loserTree[winnerTree[3]]) {
						loserTree[1] = winnerTree[2];
						winnerTree[1] = winnerTree[3];
						loserTree[0] = winnerTree[3];
					}
					else {
						loserTree[1] = winnerTree[3];
						winnerTree[1] = winnerTree[2];
						loserTree[0] = winnerTree[2];
					}
				}
			}
		}
		int tempWinner = loserTree[0];

		for (int i = 0; i < 3; i++) {
			int parent = tempWinner / 2;

			for (int j = 0; j < i; j++) {
				parent /= 2;
			}

			if (loserTreeStatus[loserTree[parent]] == false) {
				if (loserTreeStatus[tempWinner] == false) {
					// 不用操作
				}
				else {
					// tempWinner自动上升，也不用操作
				}
			}
			else if (loserTree[tempWinner] > loserTree[loserTree[parent]] || loserTreeStatus[tempWinner] == false) {
				// loser更新
				int temp = tempWinner;
				tempWinner = loserTree[parent];
				loserTree[parent] = temp;
			}
		}

		loserTree[0] = tempWinner;
	}

	// 所有数全部读入，处理最后的outputBuffer、run和败者树中剩余的数
	if (currentOutputBuffer == 1)
	{
		outputThread2.join();
		memoryOutputT(run, outputBuffer1);
	}
	else
	{
		outputThread1.join();
		memoryOutputT(run, outputBuffer2);
	}

	disk.push(run);
	run.clear();

	if (outputThread1.joinable())
		outputThread1.join();

	if (outputThread2.joinable())
		outputThread2.join();

	if (inputThread1.joinable())
		inputThread1.join();

	if (inputThread2.joinable())
		inputThread2.join();

	for (int j = 0; j < 16; j++)
	{
		loserTreeStatus[j] = true;
	}

	// 败者树初始化
	for (int i = 4; i < 8; i++)
	{
		if (loserTree[2 * i] > loserTree[2 * i + 1])
		{
			loserTree[i] = 2 * i;
			winnerTree[i] = 2 * i + 1;
		}
		else
		{
			loserTree[i] = 2 * i + 1;
			winnerTree[i] = 2 * i;
		}
	}

	for (int i = 2; i < 4; i++)
	{
		if (loserTree[winnerTree[2 * i]] > loserTree[winnerTree[2 * i + 1]])
		{
			loserTree[i] = winnerTree[2 * i];
			winnerTree[i] = winnerTree[2 * i + 1];
		}
		else
		{
			loserTree[i] = winnerTree[2 * i + 1];
			winnerTree[i] = winnerTree[2 * i];
		}
	}

	if (loserTree[winnerTree[2]] > loserTree[winnerTree[3]])
	{
		loserTree[1] = winnerTree[2];
		winnerTree[1] = winnerTree[3];
		loserTree[0] = winnerTree[3];
	}
	else
	{
		loserTree[1] = winnerTree[3];
		winnerTree[1] = winnerTree[2];
		loserTree[0] = winnerTree[2];
	}

	for (int i = 0; i < 8; i++)
	{
		// 胜者输出与新值输入
		int winner = loserTree[loserTree[0]];
		if (currentOutputBuffer == 1)
		{
			outputBuffer1.push_back(winner);
		}
		else
		{
			outputBuffer2.push_back(winner);
		}
		loserTree[loserTree[0]] = 1001;// 模拟最大值+1

		int tempWinner = loserTree[0];

		for (int i = 0; i < 3; i++)
		{
			int parent = tempWinner / 2;

			for (int j = 0; j < i; j++)
			{
				parent /= 2;
			}

			if (loserTreeStatus[loserTree[parent]] == false)
			{
				if (loserTreeStatus[tempWinner] == false)
				{
					// 不用操作
				}
				else
				{
					// tempWinner自动上升，也不用操作
				}
			}
			else if (loserTree[tempWinner] > loserTree[loserTree[parent]] || loserTreeStatus[tempWinner] == false)
			{
				// loser更新
				int temp = tempWinner;
				tempWinner = loserTree[parent];
				loserTree[parent] = temp;
			}
		}

		loserTree[0] = tempWinner;
	}

	if (currentOutputBuffer == 1)
	{
		memoryOutput(run, outputBuffer1);
	}
	else
	{
		memoryOutput(run, outputBuffer2);
	}

	disk.push(run);
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
			{
				disk_action_amount_run++;
				inputBuffer1 = memoryInput(tempDisk1, inputBufferSize);
			}
			if (!tempDisk2.empty() && inputBuffer2.empty())
			{
				disk_action_amount_run++;
				inputBuffer2 = memoryInput(tempDisk2, inputBufferSize);
			}

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

		disk_action_amount_run++;
		memoryOutput(outputDisk, outputBuffer);
	}

	return outputDisk;
}

void external_sort(string inFileName, int inputBufferSize, priority_queue<vector<int>, vector<vector<int>>, Compare>& disk, string outFileName, int outputBufferSize)
{
	segmentInitial(inFileName, inputBufferSize, outputBufferSize, disk, outFileName);

	/*int temp = 0;
	for (int i = 0; i < 31; i++)
	{
		temp += disk.top().size();
		disk.pop();
	}*/

	while (disk.size() > 1)
	{
		vector<int> tempDisk1 = disk.top();
		disk.pop();
		vector<int> tempDisk2 = disk.top();
		disk.pop();

		vector<int> outputDisk = EXT_merge_sort(tempDisk1, tempDisk2, inputBufferSize, outputBufferSize);

		disk.push(outputDisk);
	}

	vector<int> result = disk.top();

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
	int N = 5000; // 对500个整数进行外排

	string inFileName = "input.txt"; // 用txt文件模拟磁盘
	priority_queue<vector<int>, vector<vector<int>>, Compare> disk; // 用vector队列模拟磁盘
	string outFileName = "output.txt"; // 用txt文件模拟磁盘

	// 不要用文件模拟过程中的磁盘了，还是用vector队列来模拟运算过程中的磁盘

	// 用priority_queue来实现哈夫曼顺序merge

	int outputBufferSize = 20;
	int inputBufferSize = 10;

	data_generation(N, inFileName);

	external_sort(inFileName, inputBufferSize, disk, outFileName, outputBufferSize);

	disk_action_amount_pre = disk_action_amount_sum - disk_action_amount_run;

	cout << "Disk action amount sum: " << disk_action_amount_sum << endl;
	cout << "Disk pre action amount : " << disk_action_amount_pre << endl;
	cout << "Disk run action amount : " << disk_action_amount_run << endl;
}