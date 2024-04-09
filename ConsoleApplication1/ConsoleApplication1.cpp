#include <iostream>
#include <conio.h>
#include <thread>
#include <mutex>
#include <map>
#include <vector>
#include <windows.h>
#include <random>
#include <stdlib.h>
#include <string>
#include <fstream>

using namespace std;

HANDLE closeEvent;
HANDLE mfile1;
HANDLE mfile2;
HANDLE m;
HANDLE ReadCash0;
HANDLE ReadCash1;
HANDLE ReadCash2;
HANDLE ReadCash3;

void exitAll(vector<pair<PROCESS_INFORMATION, STARTUPINFO>>& v)
{
	for (auto& i : v)
	{
		CloseHandle(i.first.hThread);
		CloseHandle(i.first.hProcess);
	}

	if (closeEvent)
		CloseHandle(closeEvent);

	CloseHandle(m);
	CloseHandle(mfile1);
	CloseHandle(mfile2);
}

void closeThread()
{
	WaitForSingleObject(m, INFINITE);
	cout << "to exit press any key and enter\n";
	ReleaseMutex(m);
	if (cin.get() != 0)
	{
		SetEvent(closeEvent);
	}
}

int main(int argc, char* argv[])
{
	int rtime = 2000;
	if (argc == 1 || (argc > 2 && !atoi(argv[1])))
	{
		cout << "there are no rand time parameter or it is incorrect, will be used 2000 ms\n";
	}
	else
	{
		rtime = atoi(argv[1]);
	}

	m = CreateMutex(NULL, FALSE, L"MUTEX");
	if (!m)
	{
		cout << "mutex is not created\n";
		return GetLastError();
	}

	mfile1 = CreateMutex(NULL, FALSE, L"FILE1");
	if (!mfile1)
	{
		cout << "mutex is not created\n";
		return GetLastError();
	}

	mfile2 = CreateMutex(NULL, FALSE, L"FILE2");
	if (!mfile2)
	{
		cout << "mutex is not created\n";
		return GetLastError();
	}


	ReadCash0 = CreateEvent(NULL, true, true, L"CASH0");
	ReadCash1 = CreateEvent(NULL, true, true, L"CASH1");
	ReadCash2 = CreateEvent(NULL, true, true, L"CASH2");
	ReadCash3 = CreateEvent(NULL, true, true, L"CASH3");
	if (!ReadCash0 || !ReadCash1 || !ReadCash2 || !ReadCash3)
	{
		cout << "process event is not created\n";
		return GetLastError();
	}

	closeEvent = CreateEvent(NULL, true, false, L"CLOSE");
	if (!closeEvent)
	{
		cout << "Warning!!! close event is not created, programm will be executed endlessly\n";
	}

	/*HANDLE close;
	DWORD closeID;
	close = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)closeThread, NULL, 0, &closeID);
	if (!close)
	{
		cerr << "Can't create close thread\n";
		return 0;
	}*/

	srand(time(NULL));

	vector<pair<PROCESS_INFORMATION, STARTUPINFO>> vecProcess(4);
	HANDLE lpProcessArr[4];
	int id = 0;
	for (auto& i : vecProcess)
	{
		ZeroMemory(&i.second, sizeof(STARTUPINFO));
		i.second.cb = sizeof(STARTUPINFO);
		wstring name = (wstring(L"Customer") + to_wstring(id) + L".exe" + L' ' + to_wstring(rtime));
		const wchar_t* lpName = name.c_str();
		if (!CreateProcess(NULL, (LPWSTR)lpName, NULL, NULL, FALSE, 0, NULL, NULL, &i.second, &i.first))
		{
			WaitForSingleObject(m, INFINITE);
			wcout << "Process " << lpName << " is not created\n";
			cout << "Error " << GetLastError() << '\n';
			ReleaseMutex(m);
		}
		else
		{
			lpProcessArr[id] = i.first.hProcess;
		}

		//SetSecurityInfo(i.first.hProcess, SE_KERNEL_OBJECT, PROCESS_ALL_ACCESS, NULL, NULL, NULL, NULL);

		id++;
	}

	closeThread();
	for (int i = 0; i < 4; i++)
	{
		if (lpProcessArr && lpProcessArr[i])
		{
			WaitForSingleObject(lpProcessArr[i], INFINITE);
		}
	}

	exitAll(vecProcess);

	ifstream file1("goods1.txt");
	ifstream file2("goods2.txt");

	string s;
	int val;
	vector<string> keys;
	vector<int> values;
	vector<int> cnts;
	while (file1 >> s)
	{
		keys.push_back(s);
		file1 >> val;
		values.push_back(val);

		file1.seekg(1, ios::cur);
		if (file1.peek() != 32)
		{
			int cnt;
			file1 >> cnt;
			cnts.push_back(cnt);
		}
		else cnts.push_back(0);

	}

	int i = 0;
	while (file2 >> s)
	{
		file2 >> val;
		file2.seekg(1, ios::cur);
		if (file2.peek() != 32)
		{
			int cnt;
			file2 >> cnt;
			cnts[i] += cnt;
		}

		i++;
	}

	int result = 0;
	cout << "\nmerging cashes: \n";
	for (i = 0; i < keys.size(); i++)
	{
		cout << keys[i] << '\t' << values[i] << '\t' << cnts[i] << '\n';
		result += values[i] * cnts[i];
	}

	cout << "final spendings: " << result << '\n';

	file1.close();
	file2.close();

	system("pause");
}