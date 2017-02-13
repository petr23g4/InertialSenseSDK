/*
MIT LICENSE

Copyright 2014 Inertial Sense, LLC - http://inertialsense.com

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef __CLTOOL_H__
#define __CLTOOL_H__

#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <string>

// change these includes to be the correct path for your system
#include "../../src/InertialSense.h"
#include "../../src/ISDisplay.h"
#include "../../src/ISUtilities.h"

using namespace std;

#define APP_NAME                "cltool"
#if defined(_WIN32)
#define APP_EXT                 ".exe"
#define EXAMPLE_PORT            "COM5"
#define EXAMPLE_LOG_DIR         "c:\\logs\\20170117_222549     "
#define EXAMPLE_FIRMWARE_FILE   "c:\\fw\\IS_uINS-3.hex  "
#define EXAMPLE_SPACE_1         "     "
#else
#define APP_EXT	                ""
#define EXAMPLE_PORT            "/dev/ttyS2"
#define EXAMPLE_LOG_DIR         "logs/20170117_222549             "
#define EXAMPLE_FIRMWARE_FILE   "fw/IS_uINS-3.hex    "
#define EXAMPLE_SPACE_1         "    "
#endif

typedef struct
{
	// calculated
	uint32_t logSolution;

	// parsed
	string comPort; // -c=com_port
	string bootloaderFileName; // -b=file_name
	bool replayDataLog;
	double replaySpeed;
	int displayMode;

	bool streamINS1;
	bool streamINS2;
	bool streamDualIMU;
	bool streamIMU1;
	bool streamIMU2;
	bool streamGPS;
	bool streamMag1;
	bool streamBaro;
	bool streamSol;
	bool streamSysSensors;
	bool streamDThetaVel;

	bool enableLogging;
	string logPath; // -lp=path
	float maxLogSpaceMB; // -lms=max_space_mb
	uint32_t maxLogFileSize; // -lmf=max_file_size
	uint32_t maxLogMemory; // -lmm=max_memory
	bool useLogTimestampSubFolder; // -lts=1
} cmd_options_t;

extern cmd_options_t g_commandLineOptions;
extern serial_port_t g_serialPort;
extern cInertialSenseDisplay g_inertialSenseDisplay;
extern bool g_ctrlCPressed;

int cltool_serialPortSendComManager(CMHANDLE cmHandle, int pHandle, buffer_t* bufferToSend);
void cltool_setupLogger(InertialSense& inertialSenseInterface);
bool cltool_parseCommandLine(int argc, char* argv[]);
bool cltool_replayDataLog();
int cltool_runBootloader(const char* port, const char* fileName, const char* verifyFileName);
void cltool_outputUsage();
void cltool_setupCtrlCHandler();

#endif // __CLTOOL_H__
