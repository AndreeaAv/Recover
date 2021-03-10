#pragma once
#include <Windows.h>

using namespace std;

class CPartition {
private:
	char partitionLetter;


public:
	void setPartitionLetter(char letter);
	void readPartition();
};