/**
 * Name: LJM_StreamUtilities.c
 * Desc: Provides some basic helper functions for stream applications
 **/

#ifndef LJM_STREAM_UTILITIES
#define LJM_STREAM_UTILITIES

/**
 * Enables logging for stream purposes
 **/
void SetupStreamDebugLogging();

/**
 * Prints any scan information
 **/
void PrintScans(int numScans, int numChannels, const char **channelNames,
                const int *channelAddresses, int deviceScanBacklog, int LJMScanBacklog,
                int iteration, double *aData);

/**
 * Prints information after stream has finished
 **/
void PrintStreamConclusion(unsigned int timeStart, unsigned int timeEnd, int numReads,
                           int scansPerRead, int numChannels, int totalSkippedScans);

/**
 * Calulates how many LJM_eStreamRead calls should be done.
 * Para: numSeconds: the desired number of seconds to stream for.
 *       scanRate: the actual scan rate returned from the device.
 *           (LJM_eStreamStart returns this)
 *       scansPerRead: the ScansPerRead parameter of LJM_eStreamStart
 **/
int CalculateNumReads(int numSeconds, double scanRate, int scansPerRead);

/**
 * Checks if stream is enabled on the device, then disables it if so
 **/
void DisableStreamIfEnabled(int handle);

/**
 * Iterates through aData and returns the number of autorecovery scans.
 * Para: numInChannels, the number of stream in channels.
 *       scansPerRead, the number of scans in one LJM_eStreamRead
 *       aData, the results of one LJM_eStreamRead
 **/
int CountAndOutputNumSkippedScans(int numInChannels, int scansPerRead, double *aData);

/**
 * Prints the iteration number, and prints the backlog values if they are greater than their
 * respective thresholds.
 **/
void OutputStreamIterationInfo(int iteration, int deviceScanBacklog, int deviceScanBacklogThreshold,
                               int LJMScanBacklog, int LJMScanBacklogThreshold);

/**
 * Calculates how much sleep should be done based on how far behind stream is.
 **/
double CalculateSleepFactor(int scansPerRead, int LJMScanBacklog);

/**
 * Sleeps for approximately the expected amount of time until the next scan is
 * ready to be read.
 **/
void VariableStreamSleep(int scansPerRead, int scanRate, int LJMScanBacklog);

/**
 * Enables externally clocked stream on the device. On the T7 and T8, externally
 * clocked stream is read by pulses input to CIO3.
 **/
void SetupExternalClockStream(int handle);

/**
 * Enables FIO0 to pulse out for numPulses pulses. This is used in these
 * examples for external stream. This is especially useful for testing external
 * stream - connect a wire from FIO0 to CIO3 and call this function before
 * starting stream. numPulses should be greater than the expected number of
 * pulses needed because clock shift may occur.
 **/
void EnableFIO0PulseOut(int handle, int pulseRate, int numPulses);

/**
 * Does a printf of the message then outputs the text to the debug
 * logger. Requires C99 or later standard and LABJACK_DEBUG_LOG_OUTPUT must be
 * defined and set to 1
 **/
void PrintAndLog(const char *outputString, ...);

// Source

#ifdef LABJACK_DEBUG_LOG_OUTPUT
void PrintAndLog(const char *outputString, ...);
#endif

#endif // #define LJM_STREAM_UTILITIES