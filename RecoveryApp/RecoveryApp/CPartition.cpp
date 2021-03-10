#include <stdio.h>
#include "CPartition.h"
#include "Utils.h"

void CPartition::setPartitionLetter(char letter)
{
	partitionLetter = letter;
}

void CPartition::readPartition()
{
	TCHAR drivePath[8];
	char* buff=(char*)malloc(64*1024);
	DWORD bytesRead;
	BOOL ret = 0;
	wsprintf(drivePath, L"\\\\.\\%c:", partitionLetter);

	HANDLE PHandle = CreateFile(drivePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (PHandle == INVALID_HANDLE_VALUE) {
		printf("Source file not opened. Error %u", GetLastError());
		return;
	}

	//loop for reading partition data (multiplii de 512)
	ret = ReadFile(PHandle, buff, sizeof(buff), &bytesRead, NULL);

	//if error
	if (ret == 0) {
		LOG_ERROR("Read partition failed!");
		return;
	}


}
