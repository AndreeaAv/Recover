#pragma once
#include <WinSock2.h>
#include <Windows.h>
#include <list>
#include <string>
#include "Utils.h"

using namespace std;

class CDeviceManager {
protected:
	list<string> physicalDevices;
public:
	CDeviceManager();

	void printPhysicalDevices();
	
private:
	BOOL getPhysicalDevices();

};