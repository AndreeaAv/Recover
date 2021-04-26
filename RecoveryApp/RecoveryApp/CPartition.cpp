#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string>
#include "CPartition.h"
#include "Utils.h"
#include <algorithm>
#include <fstream>
#include <iostream>

#define BLOCK_SIZE 512

using namespace std;

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

CPartition::CPartition()
{
}

void CPartition::writeBitmap(HANDLE PHandle)
{
	std::wstring bitmapFile;
	bitmapFile = L"bitmap.bin";

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

	volumeClusters = bitmapBuff->BitmapSize;
	bytesInVolume = volumeClusters.QuadPart * intToLargeInt(4096).QuadPart;

}

void CPartition::readPartition()
{
	TCHAR drivePath[8];
	char* buff=(char*)malloc(64*1024);
	DWORD bytesRead;
	BOOL ret = 0;
	DWORD dwRet, dwErr, dwBytesRead;
	__int64 bytesCount;
	char* buffer = (char*)malloc(BLOCK_SIZE * sizeof(char));

	memset(buffer, 0, BLOCK_SIZE);
	wsprintf(drivePath, L"\\\\.\\%c:", partitionLetter);

	HANDLE PHandle = CreateFile(drivePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, NULL, NULL);

	if (PHandle == INVALID_HANDLE_VALUE) {
		printf("Source file not opened. Error %u", GetLastError());
		return;
	}

	writeBitmap(PHandle);

	//-----------------------------------------------------------------------

	//startWorkers();
	FILE* fOut = fopen("data.bin", "wb");
	std::string::size_type f;
	int count=0;

	for (bytesCount = 0;bytesCount < bytesInVolume;bytesCount+= BLOCK_SIZE)
	{
		dwRet = ReadFile(PHandle, buffer, BLOCK_SIZE, &dwBytesRead, NULL);
		if (dwRet == FALSE) {
			dwErr = GetLastError();

			switch (dwErr) {
			case ERROR_HANDLE_EOF:
				printf("End of File Reached\n");
				break;

			default:
				printf("ReadFile error\n");
			}
		}
		else {
			//printf("Bytes Read = %d\n", dwBytesRead);

			std::string tmp(buffer, dwBytesRead);
			const char docStart[] = "\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1";
			const char docStop[] = "\x05\x00\x44\x00\x6F\x00\x63\x00\x75\x00\x6D\x00\x65\x00\x6E\x00\x74\x00\x53\x00\x75\x00\x6D\x00\x6D\x00\x61\x00\x72\x00\x79\x00";
			//const char docStop[] = "\x05\x00\x44\x00\x6F\x00\x63\x00";
			std::string stop(docStop, 16);


			if (tmp.find(stop, 0) != string::npos) {
				printf("Found it!\n");
				count++;
			}			

			fwrite(buffer, sizeof(char), dwBytesRead, fOut);
			

			//putTask(makeTask(0));
		}

	}
	printf("Count: %d", count);
	fclose(fOut);
	//joinWorkerThreads();
	CloseHandle(PHandle);

}


