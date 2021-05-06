#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <conio.h>
#include "DeviceManager.h"
#include "CPartition.h"

int main() {

	CDeviceManager devMng;
	CPartition chosenPartition;
	char partLetter;

	devMng.printPhysicalDevices();
	printf("Choose partition to recover: ");
	scanf("%c", &partLetter);
	chosenPartition.setPartitionLetter(partLetter);
	chosenPartition.readAndRecover();
	printf("finished");
	_getch();
	return 0;
}