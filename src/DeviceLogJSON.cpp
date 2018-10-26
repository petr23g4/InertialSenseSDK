/*
MIT LICENSE

Copyright 2014-2018 Inertial Sense, Inc. - http://inertialsense.com

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
#include <limits.h>

#include "ISPose.h"
#include "DeviceLogJSON.h"
#include "ISLogger.h"
#include "ISDataMappings.h"

using namespace std;

bool cDeviceLogJSON::CloseAllFiles()
{
	cDeviceLog::CloseAllFiles();
	if (m_pFile != NULLPTR)
	{
		fputc(']', m_pFile);
		fclose(m_pFile);
		m_pFile = NULLPTR;
	}
	return true;
}

bool cDeviceLogJSON::GetNextItemForFile()
{
	m_jsonString.clear();
	if (m_pFile == NULLPTR)
	{
		return false;
	}
	int stack = 0;
	int c;
	int pc = 0;
	while ((c = fgetc(m_pFile)) >= 0)
	{
		if (c == '{' && pc != '\\')
		{
			stack++;
		}
		else if (c == '}' && pc != '\\')
		{
			stack--;
			if (stack == 0)
			{
                m_jsonString.append(1, c);
				break;
			}
		}

        if (stack != 0)
		{
			m_jsonString.append(1, c);
		}

		pc = c;
	}
	return (m_jsonString.size() != 0);
}


bool cDeviceLogJSON::SaveData(p_data_hdr_t* dataHdr, const uint8_t* dataBuf)
{
    cDeviceLog::SaveData(dataHdr, dataBuf);

    bool needsComma = true;

	// Create first file it it doesn't exist, return out if failure
    if (m_pFile == NULLPTR)
	{
        needsComma = false;
        if (!OpenNewSaveFile())
        {
            return false;
        }
        fputc('[', m_pFile);
	}
	else if (dataHdr->id == DID_DEV_INFO)
	{
		memcpy(&m_devInfo, dataBuf, sizeof(dev_info_t));
	}

	// Write date to file
    int nBytes = m_json.WriteDataToFile(m_pFile, *dataHdr, dataBuf, (needsComma ? ",\n" : NULLPTR));
	if (ferror(m_pFile) != 0)
	{
		return false;
	}

	// File byte size
	m_logSize += nBytes;

	if (m_logSize >= m_maxFileSize)
	{
		// Close existing file
		fputc(']', m_pFile);
		fclose(m_pFile);
		m_pFile = NULL;
		m_logSize = 0;
	}

	return true;
}


p_data_t* cDeviceLogJSON::ReadData()
{
	if (m_pFile == NULLPTR)
	{
        if (!OpenNextReadFile())
        {
            return NULLPTR;
        }
	}
	p_data_t* data = ReadDataFromFile();
    cDeviceLog::OnReadData(data);
	return data;
}


p_data_t* cDeviceLogJSON::ReadDataFromFile()
{
	if (m_pFile == NULLPTR)
	{
		assert(false);
		return NULL;
	}
    while (!GetNextItemForFile() && OpenNextReadFile()) {}
	if (m_json.StringJSONToData(m_jsonString, m_dataBuffer.hdr, m_dataBuffer.buf, _ARRAY_BYTE_COUNT(m_dataBuffer.buf)))
	{
		if (m_dataBuffer.hdr.id == DID_DEV_INFO)
		{
			memcpy(&m_devInfo, m_dataBuffer.buf, sizeof(dev_info_t));
		}
		return &m_dataBuffer;
	}
    return NULLPTR;
}


void cDeviceLogJSON::SetSerialNumber(uint32_t serialNumber)
{
	m_devInfo.serialNumber = serialNumber;
}



