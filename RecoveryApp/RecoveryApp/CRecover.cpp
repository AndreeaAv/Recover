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
	const char docStart[] = "\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1";
	const char docStop[] = "\x57\x00\x6F\x00\x72\x00\x64\x00\x44\x00\x6F\x00\x63\x00\x75\x00\x6D\x00\x65\x00\x6E\x00\x74\x00";
	std::string dStart(docStart, 8);
	std::string dStop(docStop, 25);

	files[0].type = "doc";
	files[0].startBytes = dStart;
	files[0].endBytes = dStop;
}

int CRecover::recover(char* bitmapFile, HANDLE dataHandle, char** types)
{
	DWORD dwRet, dwErr, dwBytesRead;
	int start = 0;
	long int count = 0;
	FILE* doc;
	//startWorkers();

	//to be deleted later
	FILE* fOut = fopen("data.bin", "wb");

	char* buffer = (char*)malloc(BLOCK_SIZE * sizeof(char));
	memset(buffer, 0, BLOCK_SIZE);

	char* docname = (char*)malloc(9 * sizeof(char));
	strcpy(docname, "doc");

	//Get the size of the Volume
	FILE* bitmap = fopen(bitmapFile,"rb");
	fseek(bitmap, 0L, SEEK_END);
	int bytesInVolume = (ftell(bitmap) * 8 - 1) * 4096;
	fseek(bitmap, 0L, SEEK_SET);	

	char c;
	
	while (!feof(bitmap)) {
		c = fgetc(bitmap);
		for (int i = 7; i >= 0; i--) {
			dwRet = ReadFile(dataHandle, buffer, BLOCK_SIZE, &dwBytesRead, NULL);
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
				//if (((c >> i) & 1) == 0) {
					std::string tmp(buffer, dwBytesRead);

					if (start == 1) {
						doc = fopen(docname, "ab");
						fwrite(buffer, sizeof(char), dwBytesRead, doc);
						fclose(doc);
					}

					if (tmp.find(files[0].endBytes, 0) != string::npos) {
						start = 0;
						printf("Found stop!\n");
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

					fwrite(buffer, sizeof(char), dwBytesRead, fOut);

					//putTask(makeTask(0));
				//}
			}
		}
	}


	//
	//for (__int64 bytesCount = 0; bytesCount < bytesInVolume; bytesCount += BLOCK_SIZE)
	//{
	//	count++;
	//	dwRet = ReadFile(dataHandle, buffer, BLOCK_SIZE, &dwBytesRead, NULL);
	//	if (dwRet == FALSE) {
	//		dwErr = GetLastError();

	//		switch (dwErr) {
	//		case ERROR_HANDLE_EOF:
	//			printf("End of File Reached\n");
	//			break;

	//		default:
	//			printf("ReadFile error\n");
	//		}
	//	}
	//	else {
	//		std::string tmp(buffer, dwBytesRead);

	//		if (start == 1) {
	//			doc = fopen(docname, "ab");
	//			fwrite(buffer, sizeof(char), dwBytesRead, doc);
	//			fclose(doc);
	//		}

	//		if (tmp.find(files[0].endBytes, 0) != string::npos) {
	//			printf("Found stop!\n");
	//			start = 0;
	//		}

	//		if (tmp.find(files[0].startBytes, 0) != string::npos) {
	//			char index[10];
	//			_itoa(count, index, 10);
	//			strcat(docname, index);
	//			strcat(docname, ".doc");

	//			doc = fopen(docname, "wb");
	//			fwrite(buffer, sizeof(char), dwBytesRead, doc);
	//			fclose(doc);
	//			printf("Found start!\n");
	//			start = 1;
	//		}

	//		fwrite(buffer, sizeof(char), dwBytesRead, fOut);

	//		//putTask(makeTask(0));
	//	}

	//}
	
	fclose(fOut);
	//joinWorkerThreads();


	return 0;
}
