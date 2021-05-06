#pragma once
#include "CRecover.h"
#include <Windows.h>
//#include "Workers.h"

using namespace std;

class CPartition {
private:
	char partitionLetter;
	std::wstring bitmapFile;



public:
	CPartition();
	void setPartitionLetter(char letter);
	void readAndRecover();
	void writeBitmap(HANDLE partitionHandle);

	//static Task makeTask(int i);
	static void search(void* data, int thread_id);
};