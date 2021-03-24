#include <stdio.h>
#include <string>
#include "CPartition.h"
#include "Utils.h"

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

void CPartition::readPartition()
{
	TCHAR drivePath[8];
	char* buff=(char*)malloc(64*1024);
	DWORD bytesRead;
	BOOL ret = 0;
	wsprintf(drivePath, L"\\\\.\\%c:", partitionLetter);

	HANDLE PHandle = CreateFile(drivePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, NULL, NULL);

	if (PHandle == INVALID_HANDLE_VALUE) {
		printf("Source file not opened. Error %u", GetLastError());
		return;
	}

	//--------------------------------------------------------------------------------
	OVERLAPPED ov;
	HANDLE hFile;
	HANDLE hEvent;
	DWORD dwRet, dwErr, dwBytesRead;
	char* buffer = (char*) malloc(4096 * 1024 * sizeof(char));
	BOOL retFile;
	DWORD bytesReturned, bytesWritten;
	VOLUME_BITMAP_BUFFER* bitmapBuff = NULL;
	UINT32 BitmapSize;
	LARGE_INTEGER volumeClusters;
	STORAGE_PROPERTY_QUERY inProperty;
	inProperty.PropertyId = StorageDeviceProperty;
	inProperty.QueryType = PropertyStandardQuery;
	STARTING_LCN_INPUT_BUFFER startingLcn;
	startingLcn.StartingLcn.QuadPart = 0;

	BitmapSize = 4096*100 + sizeof(LARGE_INTEGER) * 2;
	bitmapBuff = (VOLUME_BITMAP_BUFFER*)malloc(BitmapSize);

	std::wstring bitmapFile;
	bitmapFile = L"dummy_bitmap.bin";
	hFile = CreateFile(bitmapFile.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);

	while (TRUE) {
		ret = DeviceIoControl(PHandle, FSCTL_GET_VOLUME_BITMAP, &inProperty, sizeof(STORAGE_PROPERTY_QUERY), bitmapBuff, BitmapSize, &bytesReturned, NULL);

		//if error
		if (ret == 0 && GetLastError() != ERROR_MORE_DATA) {
			LOG_ERROR("DeviceIoControl failed!");
			return;
		}
		bytesReturned -= sizeof(LARGE_INTEGER) * 2;
		retFile = WriteFile( hFile, bitmapBuff->Buffer, bytesReturned, &bytesWritten, NULL);

		if (TRUE == ret) {
			break;
		}
		startingLcn.StartingLcn.QuadPart += bytesReturned * 8;

	}
	CloseHandle(hFile);
	
	volumeClusters = bitmapBuff->BitmapSize;
	__int64 bytesInVolume = volumeClusters.QuadPart * intToLargeInt(4096).QuadPart;

	ZeroMemory(&ov, sizeof(ov));
	memset(buffer, 0, 4096 * 1024);

	hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	ov.hEvent = hEvent;

	
	__int64 bytesCount;
	for (bytesCount = 0;bytesCount < bytesInVolume;bytesCount+= 4096 * 1024)
	{
		dwRet = ReadFile(PHandle, buffer, 4096 * 1024, &dwBytesRead, NULL);
		if (dwRet == FALSE) {
			dwErr = GetLastError();

			switch (dwErr) {
			case ERROR_HANDLE_EOF:
				printf("End of File Reached\n");
				break;

			case ERROR_IO_PENDING:
				printf("Async IO not finished immediately\n");
				Sleep(1);
				//dwRet = GetOverlappedResult(ov.hEvent, &ov, &dwBytesRead, TRUE);
				printf("nRead = %d\n", dwBytesRead);
				break;

			default:
				printf("ReadFile error\n");
			}
		}
		else {
			printf("Async IO finished immediately\n");
			printf("nRead = %d\n", dwBytesRead);
		}
	}


	dwRet = CloseHandle(PHandle);

}
