/*
MIT LICENSE

Copyright 2014 Inertial Sense, LLC - http://inertialsense.com

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <ctime>
#include <string>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "DeviceLog.h"
#include "ISLogger.h"

using namespace std;

void cDeviceLog::InitDeviceForWriting(int pHandle, std::string timestamp, std::string directory, uint64_t maxDiskSpace, uint32_t maxFileSize, uint32_t chunkSize)
{
	m_pHandle = pHandle;
	m_timeStamp = timestamp;
	m_directory = directory;
	m_fileCount = 0;
	m_maxDiskSpace = maxDiskSpace;
	m_maxFileSize = maxFileSize;
	m_maxChunkSize = chunkSize;
	m_logSize = 0;
}


void cDeviceLog::InitDeviceForReading()
{
	m_fileSize = 0;
	m_logSize = 0;
	m_fileCount = 0;
}


bool cDeviceLog::OpenWithSystemApp()
{
#if defined(_WIN32)
#include <windows.h>

	std::wstring stemp = std::wstring(m_fileName.begin(), m_fileName.end());
	LPCWSTR filename = stemp.c_str();
	ShellExecuteW(0, 0, filename, 0, 0, SW_SHOW);

#elif !defined(PLATFORM_IS_LINUX) && (TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE || TARGET_OS_MAC || __linux || __unix__ )

#endif

	return true;
}


bool cDeviceLog::SetupReadInfo(const string& directory, const string& serialNum, const string& timeStamp)
{
	m_directory = directory;
	m_fileCount = 0;
	m_timeStamp = timeStamp;
	m_fileNames.clear();
	vector<file_info_t> fileInfos;
	SetSerialNumber((uint32_t)strtoul(serialNum.c_str(), NULL, 10));
	cISLogger::GetDirectorySpaceUsed(directory, string("[\\/\\\\]" IS_LOG_FILE_PREFIX) + serialNum + "_", fileInfos, false, false);
	if (fileInfos.size() != 0)
	{
		m_fileName = fileInfos[0].name;
		for (size_t i = 0; i < fileInfos.size(); i++)
		{
			m_fileNames.push_back(fileInfos[i].name);
		}
	}
	return true;
}


bool cDeviceLog::OpenNewSaveFile()
{
	// Close existing file
	if (m_pFile)
	{
		fclose(m_pFile);
	}

	// Ensure directory exists
	if (m_directory.empty())
	{
		return false;
	}

	// create directory
	_MKDIR(m_directory.c_str());

	// clear out space if we need to
	if (m_maxDiskSpace != 0)
	{
		vector<file_info_t> files;
		uint64_t spaceUsed = cISLogger::GetDirectorySpaceUsed(m_directory.c_str(), files);
		unsigned int index = 0;

		// clear out old files until we have space
		while (spaceUsed > m_maxDiskSpace && index < files.size())
		{
			spaceUsed -= files[index].size;
			remove(files[index++].name.c_str());
		}
	}

	// Open new file
	m_fileCount++;
	uint32_t serNum = m_devInfo.serialNumber;
	if (!serNum)
	{
		serNum = m_pHandle;
	}
	string fileName = GetNewFileName(serNum, m_fileCount, NULL);
	m_pFile = fopen(fileName.c_str(), "wb");
	m_fileSize = 0;

	if (m_pFile)
	{
#if LOG_DEBUG_WRITE
		printf("Opened save file: %s\n", filename.str().c_str());
#endif
		return true;
	}
	else
	{
#if LOG_DEBUG_WRITE
		printf("FAILED to open save file: %s\n", filename.str().c_str());
#endif
		return false;
	}
}


bool cDeviceLog::OpenNextReadFile()
{
	// Close file if open
	if (m_pFile)
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}

	if (m_fileCount == m_fileNames.size())
	{
		return false;
	}
	
	m_fileName = m_fileNames[m_fileCount++];
	m_pFile = fopen(m_fileName.c_str(), "rb");

	if (m_pFile)
	{

#if LOG_DEBUG_READ
		printf("File opened: %s\n", m_fileName.c_str());
#endif
		return true;
	}
	else
	{
#if LOG_DEBUG_READ
		printf("FAILED to open file: %s\n", m_fileName.c_str());
#endif
		return false;
	}
}

string cDeviceLog::GetNewFileName(uint32_t serialNumber, uint32_t fileCount, const char* suffix)
{
	// file name 
	ostringstream filename;
	filename << m_directory << "/" << IS_LOG_FILE_PREFIX <<
		serialNumber << "_" <<
		m_timeStamp << "_" <<
		setfill('0') << setw(4) << (fileCount % 10000) <<
		(suffix == NULL || *suffix == 0 ? "" : string("_") + suffix) <<
		LogFileExtention();
	return filename.str();
}


void cDeviceLog::SetDeviceInfo(const dev_info_t *info)
{
	if (info == NULL)
	{
		return;
	}
	m_devInfo = *info;
	SetSerialNumber(info->serialNumber);
}




