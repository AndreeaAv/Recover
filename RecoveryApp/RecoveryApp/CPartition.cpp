#define _CRT_SECURE_NO_WARNINGS
#pragma once
#include <stdio.h>
#include <string>
#include "CPartition.h"
#include "Utils.h"
#include <algorithm>
#include <fstream>
#include <iostream>




//using namespace std;

LARGE_INTEGER intToLargeInt(int i) {
	LARGE_INTEGER li;
	li.QuadPart = i;
	return li;
}

void CPartition::setPartitionLetter(char letter)
{
	partitionLetter = letter;
}


void search(void* data, int thread_id);

/*
Task CPartition::makeTask(int i)
{
	Task task;
	int* newData = (int*)malloc(sizeof(int));
	newData[0] = i;
	task.data = newData;
	task.runTask = search;
	return task;
}

void CPartition::search(void* data, int thread_id)
{
	int task_id = *(int*)data;
	if (task_id > 8) {
		forceShutDownWorkers();
		return;
	}

	printf("Something %i from thread %i\n", task_id, thread_id);
	putTask(makeTask(task_id + 1));
}
*/
CPartition::CPartition()
{
	bitmapFile = L"bitmap.bin";
}

void CPartition::writeBitmap(HANDLE PHandle)
{
	HANDLE bmFile;
	BOOL retFile;
	BOOL ret;
	DWORD bytesReturned, bytesWritten;
	VOLUME_BITMAP_BUFFER* bitmapBuff = NULL;
	UINT32 BitmapSize;
	STORAGE_PROPERTY_QUERY inProperty;

	inProperty.PropertyId = StorageDeviceProperty;
	inProperty.QueryType = PropertyStandardQuery;
	STARTING_LCN_INPUT_BUFFER startingLcn;
	startingLcn.StartingLcn.QuadPart = 0;

	BitmapSize = 4096 * 100 + sizeof(LARGE_INTEGER) * 2;
	bitmapBuff = (VOLUME_BITMAP_BUFFER*)malloc(BitmapSize);

	//Create bitmap File
	bmFile = CreateFile(bitmapFile.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);

	//Write bitmap File
	while (TRUE) {
		ret = DeviceIoControl(PHandle, FSCTL_GET_VOLUME_BITMAP, &inProperty, sizeof(STORAGE_PROPERTY_QUERY), bitmapBuff, BitmapSize, &bytesReturned, NULL);

		//if error
		if (ret == 0 && GetLastError() != ERROR_MORE_DATA) {
			LOG_ERROR("DeviceIoControl failed!");
			return;
		}

		bytesReturned -= sizeof(LARGE_INTEGER) * 2;
		retFile = WriteFile(bmFile, bitmapBuff->Buffer, bytesReturned, &bytesWritten, NULL);

		if (TRUE == retFile) {
			break;
		}

		startingLcn.StartingLcn.QuadPart += bytesReturned * 8;

	}
	CloseHandle(bmFile);

}

void CPartition::readAndRecover()
{
	TCHAR drivePath[8];
	wsprintf(drivePath, L"\\\\.\\%c:", partitionLetter);

	HANDLE PHandle = CreateFile(drivePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, NULL, NULL);

	if (PHandle == INVALID_HANDLE_VALUE) {
		printf("Source file not opened. Error %u", GetLastError());
		return;
	}

	writeBitmap(PHandle);
	char* bitmapName = new char[255];
	sprintf(bitmapName, "%ls", bitmapFile.c_str());

	CRecover* recoverer = new CRecover();
	recoverer->recover(bitmapName, PHandle, 0);
	
	CloseHandle(PHandle);

}


