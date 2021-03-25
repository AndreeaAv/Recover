#pragma once
#include <Windows.h>
#include "Workers.h"

using namespace std;

class CPartition {
private:
	char partitionLetter;
	LARGE_INTEGER volumeClusters;
	__int64 bytesInVolume;



public:
	CPartition();
	void setPartitionLetter(char letter);
	void readPartition();
	void writeBitmap(HANDLE partitionHandle);

	static Task makeTask(int i);
	static void search(void* data, int thread_id);
};