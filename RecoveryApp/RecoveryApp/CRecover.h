#include <Windows.h>
#include <string>
#include "CPartition.h"
#pragma once



struct file {
	std::string type;
	std::string startBytes;
	std::string endBytes;
};

class CRecover {
private:
	
public:
	//To do: Citeste tipurile de fisier si semnaturile din fisier
	file files[4];
	CRecover();

	int recover(CPartition chosenPartition, int types);


};