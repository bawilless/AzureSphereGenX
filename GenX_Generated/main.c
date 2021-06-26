/* Copyright (c) Microsoft Corporation. All rights reserved.
 * Licensed under the MIT License.
 *
 *   DISCLAIMER
 *
 *   The DevX library supports the Azure Sphere Developer Learning Path:
 *
 *	   1. are built from the Azure Sphere SDK Samples at https://github.com/Azure/azure-sphere-samples
 *	   2. are not intended as a substitute for understanding the Azure Sphere SDK Samples.
 *	   3. aim to follow best practices as demonstrated by the Azure Sphere SDK Samples.
 *	   4. are provided as is and as a convenience to aid the Azure Sphere Developer Learning experience.
 *
 *   DEVELOPER BOARD SELECTION
 *
 *   The following developer boards are supported.
 *
 *	   1. AVNET Azure Sphere Starter Kit.
 *     2. AVNET Azure Sphere Starter Kit Revision 2.
 *	   3. Seeed Studio Azure Sphere MT3620 Development Kit aka Reference Design Board or rdb.
 *	   4. Seeed Studio Seeed Studio MT3620 Mini Dev Board.
 *
 *   ENABLE YOUR DEVELOPER BOARD
 *
 *   Each Azure Sphere developer board manufacturer maps pins differently. You need to select the
 *      configuration that matches your board.
 *
 *   Follow these steps:
 *
 *	   1. Open CMakeLists.txt.
 *	   2. Uncomment the set command that matches your developer board.
 *	   3. Click File, then Save to save the CMakeLists.txt file which will auto generate the
 *          CMake Cache.
 *
 ************************************************************************************************/

#include "gx_declarations.h"

#include "applibs_versions.h"
#include <applibs/log.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#define APPLICATION_VERSION "1.5"
const char PNP_MODEL_ID[] = "dtmi:com:example:application;1";
const char NETWORK_INTERFACE[] = "wlan0";



/// <summary>
///  Initialize gpios, device twins, direct methods, timers.
/// </summary>
static void InitPeripheralAndHandlers(void) {{
	dx_Log_Debug_Init(Log_Debug_buffer, sizeof(Log_Debug_buffer));
	gx_initPeripheralAndHandlers();

	// Uncomment the StartWatchdog when ready for production
	// StartWatchdog();
}}

/// <summary>
///     Close Timers, GPIOs, Device Twins, Direct Methods
/// </summary>
static void ClosePeripheralAndHandlers(void) {{
	dx_azureToDeviceStop();
	gx_closePeripheralAndHandlers();
	dx_timerEventLoopStop();
}}

/// <summary>
///  Main event loop for the app
/// </summary>
int main(int argc, char* argv[]) {{
	dx_registerTerminationHandler();
	if (!dx_configParseCmdLineArguments(argc, argv, &dx_config)) {{
		return dx_getTerminationExitCode();
	}}

	InitPeripheralAndHandlers();

	// Main loop
	while (!dx_isTerminationRequired()) {{
		int result = EventLoop_Run(dx_timerGetEventLoop(), -1, true);
		// Continue if interrupted by signal, e.g. due to breakpoint being set.
		if (result == -1 && errno != EINTR) {{
			dx_terminate(DX_ExitCode_Main_EventLoopFail);
		}}
	}}

	ClosePeripheralAndHandlers();
	return dx_getTerminationExitCode();
}}

// Main code blocks


/// GENX_BEGIN ID:DeferredUpdateCalculate MD5:1228ab408e101ae072880291ce561421
/// <summary>
/// Determine whether or not to defer an update
/// </summary>
/// <param name="max_deferral_time_in_minutes">The maximum number of minutes you can defer</param>
/// <returns>Return 0 to start update, return greater than zero to defer</returns>
static uint32_t DeferredUpdateCalculate_gx_handler(uint32_t max_deferral_time_in_minutes, SysEvent_UpdateType type,
	SysEvent_Status status, const char* typeDescription,
	const char* statusDescription) {

	uint32_t requested_minutes = 0;
	time_t now = time(NULL);
	struct tm* t = gmtime(&now);
	char msgBuffer[128];
	char utc[40];

	// UTC +10 would work well for devices in Australia
	t->tm_hour += 10;
	t->tm_hour = t->tm_hour % 24;

	// Set requested_minutes to zero to allow update, greater than zero to defer the update
	requested_minutes = t->tm_hour >= 1 && t->tm_hour <= 5 ? 0 : 10;

	snprintf(msgBuffer, sizeof(msgBuffer), "Utc: %s, Type: %s, Status: %s, Max defer minutes: %i, Requested minutes: %i", 
		dx_getCurrentUtc(utc, sizeof(utc)), typeDescription, statusDescription, max_deferral_time_in_minutes, requested_minutes);

	dx_deviceTwinReportState(&dt_DeferredUpdateRequest, msgBuffer);

	return requested_minutes;
}
/// GENX_END ID:DeferredUpdateCalculate


/// GENX_BEGIN ID:DeferredUpdateNotification MD5:548e7399dbcdc65dd44c4c79def80d3f
static void DeferredUpdateNotification_gx_handler(uint32_t max_deferral_time_in_minutes, SysEvent_UpdateType type,
	SysEvent_Status status, const char* typeDescription, const char* statusDescription) {

	// do something here like update a device twin before updating
	char msgBuffer[128];
	char utc[40];

	snprintf(msgBuffer, sizeof(msgBuffer), "Utc: %s, Type: %s, Status: %s, Max defer minutes: %i", dx_getCurrentUtc(utc, sizeof(utc)), typeDescription, statusDescription, max_deferral_time_in_minutes);

	dx_deviceTwinReportState(&dt_DeferredUpdateNotification, msgBuffer);
}
/// GENX_END ID:DeferredUpdateNotification

