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
	STORAGE_PROPERTY_QUERY inProperty;
	static BYTE buffer[0x1000];
	STORAGE_DEVICE_DESCRIPTOR* descriptor = (STORAGE_DEVICE_DESCRIPTOR*)buffer;
	GET_LENGTH_INFORMATION bufferLength;
	int numPartitions = 10;
	DWORD partitionBuffSize = sizeof(DRIVE_LAYOUT_INFORMATION_EX) + (numPartitions - 1) * sizeof(PARTITION_INFORMATION_EX);
	DRIVE_LAYOUT_INFORMATION_EX* partitionBuffer = (DRIVE_LAYOUT_INFORMATION_EX*)new char[partitionBuffSize];
	_FILE_SYSTEM_RECOGNITION_INFORMATION fileSystem;
	DWORD bytesReturned;
	int ct = 0;



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

		//create disk Handle
		HANDLE disk = CreateFile(deviceInterfaceDetailData->DevicePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		//if error
		if (INVALID_HANDLE_VALUE == disk) {
			LOG_ERROR("Failed to CreateFile!");
			continue;
		}

		//Get diskNumber
		ret = DeviceIoControl(disk, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &diskNumber, sizeof(STORAGE_DEVICE_NUMBER), &bytesReturned, NULL);

		//if error
		if (ret == 0) {
			LOG_ERROR("DeviceIoControl failed!");
			continue;
		}

		inProperty.PropertyId = StorageDeviceProperty;
		inProperty.QueryType = PropertyStandardQuery;

		//get Serial number and busType
		ret = DeviceIoControl(disk,			// handle to a partition
			IOCTL_STORAGE_QUERY_PROPERTY,	// dwIoControlCode
			&inProperty,					// input buffer - STORAGE_PROPERTY_QUERY structure
			sizeof(STORAGE_PROPERTY_QUERY), // size of input buffer
			buffer,							// output buffer - see Remarks
			sizeof(buffer),					// size of output buffer
			&bytesReturned,					// number of bytes returned
			NULL);							// OVERLAPPED structure

		//if error
		if (ret == 0) {
			LOG_ERROR("DeviceIoControl failed!");
			continue;
		}

		//Get size of disk
		ret = DeviceIoControl(disk, IOCTL_DISK_GET_LENGTH_INFO, &inProperty, sizeof(STORAGE_PROPERTY_QUERY), &bufferLength, sizeof(GET_LENGTH_INFORMATION), &bytesReturned, NULL);

		//if error
		if (ret == 0) {
			LOG_ERROR("DeviceIoControl failed!");
			continue;
		}

		//Get info about partitions
		ret = DeviceIoControl(disk, IOCTL_DISK_GET_DRIVE_LAYOUT_EX, NULL, 0, partitionBuffer, partitionBuffSize, &bytesReturned, NULL);

		//if error
		if (ret == 0) {
			LOG_ERROR("DeviceIoControl failed!");
			continue;
		}

		printf("PhysicalDrive %lu\n", diskNumber.DeviceNumber);
		printf("SerialNumber %s\n", buffer + descriptor->SerialNumberOffset);
		printf("Bus Type %d\n", descriptor->BusType); // 11- SATA, 17 - NVME
		printf("Size %lu GB\n", bufferLength.Length.QuadPart / (1024 * 1024 * 1024));
		printf("Partition count - %d\n", partitionBuffer->PartitionCount);

		if (partitionBuffer->PartitionStyle == 0) { //0 = MBR, 1 = GPT, 2 = not formatted
			printf("Partition MBR Signature - %d\n", partitionBuffer->Mbr.Signature);
		}
		else if (partitionBuffer->PartitionStyle == 1) {
			printf("Partition GPT StartingUsableOffset - %d\n", partitionBuffer->Gpt.StartingUsableOffset);
			printf("Partition GPT UsableLength - %d bytes\n", partitionBuffer->Gpt.UsableLength);
			printf("Partition Disk ID - %lu \n", partitionBuffer->Gpt.DiskId);
		}
		else {
			printf("Partition not formatted\n");
		}

		

		//SELECT PARTITION

		DWORD logicalDriveStringsLength;
		TCHAR logicalDriveBuffer[512];

		logicalDriveStringsLength = GetLogicalDriveStrings(512, logicalDriveBuffer);

		for (int i = 0; i < logicalDriveStringsLength; i += 4)
		{
			TCHAR currentDriveLetter;
			TCHAR drivePath[8];

			currentDriveLetter = logicalDriveBuffer[i];
			wsprintf(drivePath, L"\\\\.\\%c:", currentDriveLetter);


			DWORD volumeExtentsBufferSize = sizeof(VOLUME_DISK_EXTENTS) + (4 * sizeof(DISK_EXTENT));
			VOLUME_DISK_EXTENTS* volumeExtentBuffer = (VOLUME_DISK_EXTENTS*)malloc(volumeExtentsBufferSize);

			TCHAR drivePath2[8];
			wsprintf(drivePath2, L"%c:\\", currentDriveLetter);
			UINT driveType = GetDriveType(drivePath2);


			if (driveType == 5)
				continue;

			HANDLE CHandle = CreateFile(
				drivePath,
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL);

			if (!DeviceIoControl((HANDLE)CHandle,     // handle to device
				IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, // dwIoControlCode
				NULL,                                 // lpInBuffer
				0,                                    // nInBufferSize
				volumeExtentBuffer,                   // output buffer
				volumeExtentsBufferSize,              // size of output buffer
				&bytesReturned,						  // number of bytes returned
				NULL								  // OVERLAPPED structure
			))
			{
				if (CHandle != INVALID_HANDLE_VALUE) {
					CloseHandle(CHandle);
				}
			}


			if (CHandle != INVALID_HANDLE_VALUE)
				CloseHandle(CHandle);


			/* Check if volume is on current partition */
			int ok = 1;
			for (ct = 0; ct < partitionBuffer->PartitionCount && ok; ct++)
			{
				if (volumeExtentBuffer->NumberOfDiskExtents <= 1) {
					if (volumeExtentBuffer->Extents[0].DiskNumber == diskNumber.DeviceNumber)
					{
						if (volumeExtentBuffer->Extents[0].StartingOffset.QuadPart >= partitionBuffer->PartitionEntry[ct].StartingOffset.QuadPart &&
							volumeExtentBuffer->Extents[0].StartingOffset.QuadPart <= partitionBuffer->PartitionEntry[ct].StartingOffset.QuadPart + partitionBuffer->PartitionEntry[ct].PartitionLength.QuadPart)
						{
							ok = 0;
							switch (driveType)
							{
							case 0: printf("Drive %S is type %d - Cannot be determined.\n", drivePath2, driveType);
								break;
							case 1: printf("Drive %S is type %d - Invalid root path/Not available.\n", drivePath2, driveType);
								break;
							case 2: printf("Drive %S is type %d - Removable.\n", drivePath2, driveType);
								break;
							case 3: printf("Drive %S is type %d - Fixed.\n", drivePath2, driveType);
								break;
							case 4: printf("Drive %S is type %d - Network.\n", drivePath2, driveType);
								break;
							case 5: printf("Drive %S is type %d - CD-ROM.\n", drivePath2, driveType);
								break;
							case 6: printf("Drive %S is type %d - RAMDISK.\n", drivePath2, driveType);
								break;
							default: "Unknown value!\n";
							}
						}
					}
				}
				else
				{
					LOG_ERROR("NUMBER OF EXTENTS > 1, NOT SUPPORTED YET!");
				}
			}
			free(volumeExtentBuffer);
		}
		printf("\n\n");
	}
}

BOOL CDeviceManager::getPhysicalDevices()
{
	return 0;
}
