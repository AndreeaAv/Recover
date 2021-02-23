#include <iostream>
#include <conio.h>
#include "DeviceManager.h"

int main() {

	CDeviceManager devMng;

	devMng.printPhysicalDevices();

	_getch();
	return 0;
}