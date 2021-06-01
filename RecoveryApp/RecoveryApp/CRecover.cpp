#define _CRT_SECURE_NO_WARNINGS
#include "CRecover.h";
#include "Utils.h"
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;


CRecover::CRecover()
{
	//doc
	const char docStart[] = "\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1";
	const char docStop[] = "\x57\x00\x6F\x00\x72\x00\x64\x00\x44\x00\x6F\x00\x63\x00\x75\x00\x6D\x00\x65\x00\x6E\x00\x74\x00";
	std::string dStart(docStart, 8);
	std::string dStop(docStop, 25);

	files[0].type = "doc";
	files[0].startBytes = dStart;
	files[0].endBytes = dStop;

	//pdf
	const char pdfStart[] = "\x25\x50\x44\x46\x2D";
	const char pdfStop[] = "\x25\x25\x45\x4F\x46";
	std::string pStart(docStart, 5);
	std::string pStop(docStop, 5);

	files[1].type = "pdf";
	files[1].startBytes = pStart;
	files[1].endBytes = pStop;

}

int CRecover::recover(CPartition chosenPartition, int types)
{
	DWORD dwRet, dwErr, dwBytesRead;
	int start = 0;
	long int count = 0;
	FILE* doc;
	char* bitmapName = new char[255];
	sprintf(bitmapName, "%ls", chosenPartition.bitmapFile.c_str());

	TCHAR drivePath[8];
	wsprintf(drivePath, L"\\\\.\\%c:", chosenPartition.partitionLetter);

	HANDLE PHandle = CreateFile(drivePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, NULL, NULL);

	if (PHandle == INVALID_HANDLE_VALUE) {
		printf("Source file not opened. Error %u", GetLastError());
		return -1;
	}
	//startWorkers();

	//to be deleted later
	FILE* fOut = fopen("data.bin", "wb");

	char* buffer = (char*)malloc(BLOCK_SIZE * sizeof(char));
	memset(buffer, 0, BLOCK_SIZE);

	char* docname = (char*)malloc(9 * sizeof(char));
	strcpy(docname, "doc");

	//Get the size of the Volume
	FILE* bitmap = fopen(bitmapName,"rb");
	fseek(bitmap, 0L, SEEK_END);
	int bytesInVolume = (ftell(bitmap) * 8 - 1) * 4096;
	fseek(bitmap, 0L, SEEK_SET);	

	char c;
	
	while (c = fgetc(bitmap) != EOF) {
		
		for (int i = 7; i >= 0; i--) {
			dwRet = ReadFile(PHandle, buffer, BLOCK_SIZE, &dwBytesRead, NULL);
			count++;
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
				//if (!((c >> i) & 1)) {
					std::string tmp(buffer, dwBytesRead);

					if (start == 1) {
						doc = fopen(docname, "ab");
						fwrite(buffer, sizeof(char), dwBytesRead, doc);
						fclose(doc);
					}

					if (tmp.find(files[0].endBytes, 0) != string::npos) {
						start = 0;
						printf("Found stop!\n");
						strcpy(docname, "doc");
					}

					if (tmp.find(files[0].startBytes, 0) != string::npos) {
						char index[10];
						_itoa(count, index, 10);
						strcat(docname, index);
						strcat(docname, ".doc");

						doc = fopen(docname, "wb");
						fwrite(buffer, sizeof(char), dwBytesRead, doc);
						fclose(doc);
						printf("Found start!\n");
						start = 1;
					}
				//}
				fwrite(buffer, sizeof(char), dwBytesRead, fOut);
				//putTask(makeTask(0));
			}
		}
	}	
	fclose(fOut);
	CloseHandle(PHandle);
	//joinWorkerThreads();
	return 0;
}
