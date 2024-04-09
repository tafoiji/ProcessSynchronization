#include <iostream>
#include <fstream>
#include <windows.h>
#include <vector>
#include <map>
#include <random>
#include <string>

using namespace std;

HANDLE m;
HANDLE mfile1;
HANDLE mfile2;
HANDLE closeEvent;
HANDLE ReadCash0;
HANDLE ReadCash1;
HANDLE ReadCash2;
HANDLE ReadCash3;

int rtime;

int intRand(const int min, const int max)
{
	random_device rd;
	mt19937 generator(rd());
	uniform_int_distribution<int> distribution(min, max);
	return distribution(generator);
}

void t(fstream& file1, fstream& file2, vector<string>& expences, vector<streampos>& pos1, vector<streampos>& pos2)
{
	//srand(time(NULL)); (rand() % (rtime + 1)), rand() % expences.size()
	while (true)
	{
		//this_thread::sleep_for(chrono::milliseconds(intRand(0, rtime)));
		int temp = intRand(0, rtime);
		if (closeEvent && WaitForSingleObject(closeEvent, temp) == WAIT_OBJECT_0)
		{
			break;
		}

		int nfile = intRand(1, 2);
		int rkey = intRand(0, (int)expences.size() - 1);
		WaitForSingleObject(m, INFINITE);
		cout << "customer2 appealed to " << expences[rkey] << " in cash " << nfile << '\n';
		ReleaseMutex(m);
		if (nfile == 1)
		{
			WaitForSingleObject(mfile1, INFINITE);
			file1.clear();
			file1.seekg(pos1[rkey] + streamoff(1));
			if (file1.peek() == 32)
			{
				file1.seekg(pos1[rkey]);
				file1 << " 1";
				file1.flush();
			}
			else
			{
				file1.seekg(pos1[rkey]);
				int cnt;
				file1 >> cnt;
				file1.seekg(pos1[rkey]);
				file1 << ' ' << cnt + 1;
			}

			file1.flush();
			ReleaseMutex(mfile1);
		}
		else
		{
			WaitForSingleObject(mfile2, INFINITE);
			file2.clear();
			file2.seekg(pos2[rkey] + streamoff(1));
			if (file2.peek() == 32)
			{
				file2.seekg(pos2[rkey]);
				file2 << " 1";
				file2.flush();
			}
			else
			{
				file2.seekg(pos2[rkey]);
				int cnt;
				file2 >> cnt;
				file2.seekg(pos2[rkey]);
				file2 << ' ' << cnt + 1;
			}

			file2.flush();
			ReleaseMutex(mfile2);
		}
	}
}

int main(int argc, char* argv[])
{
	m = OpenMutex(SYNCHRONIZE, FALSE, L"MUTEX");
	mfile1 = OpenMutex(SYNCHRONIZE, FALSE, L"FILE1");
	mfile2 = OpenMutex(SYNCHRONIZE, FALSE, L"FILE2");
	if (!m || !mfile1 || !mfile2)
	{
		cout << "cannot open mutex\n";
		return GetLastError();
	}

	WaitForSingleObject(m, INFINITE);
	cout << "Customer2 started\n";
	//cout << "rtime " << argv[1] << '\n';
	ReleaseMutex(m);

	closeEvent = OpenEvent(EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, L"CLOSE");
	if (closeEvent == NULL)
	{
		WaitForSingleObject(m, INFINITE);
		cout << "Warning! Cannot open close event. Programm will be executed endlessly\n";
		ReleaseMutex(m);
	}

	ReadCash2 = OpenEvent(EVENT_MODIFY_STATE, FALSE, L"CASH2");
	if (ReadCash2)
		ResetEvent(ReadCash0);
	ReadCash1 = OpenEvent(SYNCHRONIZE, FALSE, L"CASH1");
	ReadCash0 = OpenEvent(SYNCHRONIZE, FALSE, L"CASH0");
	ReadCash3 = OpenEvent(SYNCHRONIZE, FALSE, L"CASH3");
	if (!ReadCash0 || !ReadCash1 || !ReadCash2 || !ReadCash3)
	{
		WaitForSingleObject(m, INFINITE);
		cout << "process event is not opened\n";
		ReleaseMutex(m);
		return GetLastError();
	}

	rtime = atoi(argv[1]);

	fstream file1("goods1.txt", ios::in | ios::out);
	fstream file2("goods2.txt", ios::in | ios::out);

	if (!file1 || !file2)
	{
		std::cerr << "Failed to open file" << std::endl;
		return 1;
	}

	vector<streampos> positions1;
	vector<string> keys;
	string s;

	while (file1 >> s)
	{
		keys.push_back(s);
		int val;
		file1 >> val;
		positions1.push_back(file1.tellg());

		file1.seekg(1, ios::cur);
		if (file1.peek() != 32)
		{
			int cnt;
			file1 >> cnt;
		}

	}

	vector<streampos> positions2;
	while (file2 >> s)
	{
		int val;
		file2 >> val;
		positions2.push_back(file2.tellg());

		file2.seekg(1, ios::cur);
		if (file2.peek() != 32)
		{
			int cnt;
			file2 >> cnt;
		}
	}

	SetEvent(ReadCash2);
	WaitForSingleObject(ReadCash1, INFINITE);
	WaitForSingleObject(ReadCash0, INFINITE);
	WaitForSingleObject(ReadCash3, INFINITE);

	t(file1, file2, keys, positions1, positions2);

	CloseHandle(m);
	CloseHandle(mfile1);
	CloseHandle(mfile2);
	if (closeEvent)
		CloseHandle(closeEvent);
	CloseHandle(ReadCash0);
	CloseHandle(ReadCash1);
	CloseHandle(ReadCash2);
	CloseHandle(ReadCash3);



	file1.close();
	file2.close();
}
