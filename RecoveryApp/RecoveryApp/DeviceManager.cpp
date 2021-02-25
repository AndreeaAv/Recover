#include "DeviceManager.h"
#include <SetupAPI.h>
#include <winioctl.h>
#pragma comment (lib, "Setupapi.lib")


CDeviceManager::CDeviceManager()
{
	getPhysicalDevices();
}

void CDeviceManager::printPhysicalDevices()
{
	HDEVINFO diskDevices;
	SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA deviceInterfaceDetailData;
	DWORD devNumber = 0;	//number of devices
	DWORD size;
	BOOL ret = FALSE;
	STORAGE_DEVICE_NUMBER diskNumber;
	STORAGE_DEVICE_DESCRIPTOR properties;
	DWORD bytesReturned;

	diskDevices = SetupDiGetClassDevs(
		&GUID_DEVINTERFACE_DISK,
		NULL,
		NULL,
		DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
	);

	//TODO: Log error on diskDevices

	ZeroMemory(&deviceInterfaceData, sizeof(SP_DEVICE_INTERFACE_DATA));
	deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	while (SetupDiEnumDeviceInterfaces(diskDevices, NULL, &GUID_DEVINTERFACE_DISK, devNumber, &deviceInterfaceData)) {
		devNumber++;

		//First call of SetupDiGetDeviceInterfaceDetail gets the size, the second call gets the information
		ret = SetupDiGetDeviceInterfaceDetail(diskDevices, &deviceInterfaceData, NULL, 0, &size, NULL);
		
		//if error
		if ((ret == FALSE) && (GetLastError() != ERROR_INSUFFICIENT_BUFFER)) {
			LOG_INFO("SetupDiGetDeviceInterfaceDetail failed!");
			continue;
		}

		deviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(size);

		ZeroMemory(deviceInterfaceDetailData, size);
		deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		ret = SetupDiGetDeviceInterfaceDetail(diskDevices, &deviceInterfaceData, deviceInterfaceDetailData, size, NULL, NULL);

		//if error
		if (ret == FALSE) {
			LOG_INFO("SetupDiGetDeviceInterfaceDetail failed!");
			continue;
		}

		HANDLE disk = CreateFile(deviceInterfaceDetailData->DevicePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		
		//if error
		if (INVALID_HANDLE_VALUE == disk) {
			LOG_ERROR("Failed to CreateFile!");
			continue;
		}

		ret = DeviceIoControl(disk, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &diskNumber, sizeof(STORAGE_DEVICE_NUMBER), &bytesReturned, NULL);
	
		//if error
		if (ret == 0) {
			LOG_ERROR("DeviceIoControl failed!");
			continue;
		}

		ret = DeviceIoControl(disk, IOCTL_STORAGE_QUERY_PROPERTY, NULL, 0, &properties, sizeof(STORAGE_DEVICE_DESCRIPTOR), &bytesReturned, NULL);

		//if error
		if (ret == 0) {
			LOG_ERROR("DeviceIoControl failed!");
			continue;
		}

		printf("PhysicalDrive %lu\n", diskNumber.DeviceNumber);
		printf("Device Type %d\n", properties.DeviceType);
	}
}

BOOL CDeviceManager::getPhysicalDevices()
{
	return 0;
}
