#pragma once
#include <Windows.h>
//#include "Workers.h"

using namespace std;

class CPartition {
private:


public:
	char partitionLetter;
	std::wstring bitmapFile;

	CPartition();
	void setPartitionLetter(char letter);
	void writeBitmap();

	//static Task makeTask(int i);
	//static void search(void* data, int thread_id);
};