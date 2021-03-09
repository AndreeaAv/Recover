#pragma once
#include <WinSock2.h>
#include <Windows.h>
#include <list>
#include <string>
#include "Utils.h"

using namespace std;

class CDeviceManager {
public:
	CDeviceManager();
	void printPhysicalDevices();
	
private:
	void getDiskInfo(HANDLE diskHandle);
	void getPartitionsInfo(HANDLE diskHandle);
	void checkCurrentPartition(HANDLE diskHandle,TCHAR currentDriveLetter);
	void printPartitionType(TCHAR currentDriveLetter);

};