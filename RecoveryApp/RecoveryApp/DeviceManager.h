#pragma once
#include <WinSock2.h>
#include <Windows.h>
#include <list>
#include <string>


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