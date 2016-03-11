#include "stdafx.h"
#include <stdint.h>
#include <iostream>
#include <fstream>

//typedef adapted from the header files found in epoc32/include/
typedef uint32_t TUint32;
typedef unsigned int TUint; // according to docs " is guaranteed to be at least 32 bits in all implementations." Does this holds on modern plateforms ?
typedef signed int	TInt; // is suppose to be a weird adaptive size, see comments in e32def.h
typedef int8_t TInt8;
typedef int16_t TInt16;
typedef int64_t TInt64;

//TVersion is originally a class
struct TVersion {
	TInt8 iMajor;
	TInt8 iMinor;
	TInt16 iBuild;
};

enum TCpu
{
	ECpuUnknown = 0, ECpuX86 = 0x1000, ECpuArmV4 = 0x2000, ECpuArmV5 = 0x2001, ECpuArmV6 = 0x2002, ECpuMCore = 0x4000
};

enum TProcessPriority
{
	EPriorityLow = 150,
	EPriorityBackground = 250,
	EPriorityForeground = 350,
	EPriorityHigh = 450,
	EPriorityWindowServer = 650,
	EPriorityFileServer = 750,
	EPriorityRealTimeServer = 850,
	EPrioritySupervisor = 950
};


//Taken from:
//https://web.archive.org/web/20091213035846/http://wiki.forum.nokia.com/index.php/E32Image_file_format_on_pre-Symbian_OS_9
struct E32ImageHeader
{
	TUint32	iUid1;
	TUint32	iUid2;
	TUint32	iUid3;
	TUint32	iCheck;
	TUint iSignature;
	TCpu iCpu;
	TUint iCheckSumCode;
	TUint32 iFormat;
	TVersion iVersion;
	TInt64 iTime;
	TUint iFlags;
	TInt iCodeSize;
	TInt iDataSize;
	TInt iHeapSizeMin;
	TInt iHeapSizeMax;
	TInt iStackSize;
	TInt iBssSize;
	TUint iEntryPoint;
	TUint iCodeBase;
	TUint iDataBase;
	TInt iDllRefTableCount;
	TUint iExportDirOffset;
	TInt iExportDirCount;
	TInt iTextSize;
	TUint iCodeOffset;
	TUint iDataOffset;
	TUint iImportOffset;
	TUint iCodeRelocOffset;
	TUint iDataRelocOffset;
	TProcessPriority iPriority;
};

//Warning this was build assuming little-endianess
void u8from32(uint8_t b[4], uint32_t u32) {
	b[0] = (uint8_t)u32;
	b[1] = (uint8_t)(u32 >>= 8);
	b[2] = (uint8_t)(u32 >>= 8);
	b[3] = (uint8_t)(u32 >>= 8);
}

uint32_t u32from8(uint8_t b[4]) {
	uint32_t u32 = (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | b[0];
	return u32;
}

uint16_t crc16ccitt(uint8_t (&values)[6], uint16_t initialvalue = 0, uint16_t finalxor = 0) {
	uint32_t result = 0;

	for (uint8_t b : values) {
		result ^= (b << 8);
		for (int d = 0; d < 8; d++) {
			result = result << 1;
			if (result & 0x10000) {
				result ^= 0x1021;
			}
			result &= 0xffff;
		}
	}

	return result;
}



uint32_t calculate_iCheck(E32ImageHeader& header) {
	//calculate the 16 bits CCITT-CRC seperatly on the even bytes and the odd bytes
	uint8_t iUids[12] = {};
	u8from32(iUids, header.iUid1);
	u8from32(iUids+4, header.iUid2);
	u8from32(iUids+8, header.iUid3);
	uint8_t even_bytes[] = {iUids[0], iUids[2], iUids[4], iUids[6], iUids[8], iUids[10]};
	uint8_t odd_bytes[] = {iUids[1], iUids[3], iUids[5], iUids[7], iUids[9], iUids[11]};

	uint16_t even_crc = crc16ccitt(even_bytes);
	uint32_t odd_crc = crc16ccitt(odd_bytes);

	return ((odd_crc << 16) | even_crc);
}

int main()
{
	std::ifstream is(".\something.app", std::ios::binary);
	E32ImageHeader header;
	is.read((char *)&header.iUid1, sizeof(header.iUid1));
	is.read((char *)&header.iUid2, sizeof(header.iUid2));
	is.read((char *)&header.iUid3, sizeof(header.iUid3));
	is.read((char *)&header.iCheck, sizeof(header.iCheck));
	std::cout << "Uid1: " << header.iUid1 << std::endl;
	std::cout << "Uid2: " << header.iUid2 << std::endl;
	std::cout << "Uid3: " << header.iUid3 << std::endl;
	std::cout << "Check: " << header.iCheck << std::endl;
	std::cout << "Calculated Check: " << calculate_iCheck(header) << std::endl;
	system("PAUSE");
	return 0;
}

