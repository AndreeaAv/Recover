#include <Windows.h>
#include <string>
#pragma once



struct file {
	std::string type;
	std::string startBytes;
	std::string endBytes;
};

class CRecover {
private:
	
public:
	file files[4];
	CRecover();

	int recover(char* bitmapFile, HANDLE dataHandle , char** types);


};