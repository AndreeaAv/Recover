#include <stdio.h>
#include <string>
#include "CPartition.h"
#include "Utils.h"
#include "Workers.h"

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

Task makeTask(int i)
{
	Task task;
	int* newData = (int*)malloc(sizeof(int));
	newData[0] = i;
	task.data = newData;
	task.runTask = search;
	return task;
}

void search(void* data, int thread_id)
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
	char* buffer = (char*)malloc(4096 * 1024 * sizeof(char));

	memset(buffer, 0, 4096 * 1024);
	wsprintf(drivePath, L"\\\\.\\%c:", partitionLetter);

	HANDLE PHandle = CreateFile(drivePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, NULL, NULL);

	if (PHandle == INVALID_HANDLE_VALUE) {
		printf("Source file not opened. Error %u", GetLastError());
		return;
	}

	writeBitmap(PHandle);

	//-----------------------------------------------------------------------

	startWorkers();

	for (bytesCount = 0;bytesCount < bytesInVolume;bytesCount+= 4096 * 1024)
	{
		dwRet = ReadFile(PHandle, buffer, 4096 * 1024, &dwBytesRead, NULL);
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
			printf("Bytes Read = %d\n", dwBytesRead);
			putTask(makeTask(0));
		}
	}
	joinWorkerThreads();
	CloseHandle(PHandle);

}


