#pragma once
#include <Windows.h>

using namespace std;

class CPartition {
private:
	char partitionLetter;
	LARGE_INTEGER volumeClusters;
	__int64 bytesInVolume;

public:
	void setPartitionLetter(char letter);
	void readPartition();
	void writeBitmap(HANDLE partitionHandle);
};