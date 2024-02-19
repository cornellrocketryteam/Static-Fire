/**
 * main.cpp: Streams data from the LabJack and logs to a .csv file.
 **/

#include "LJMStreamUtil.hpp"
#include "LJMUtil.hpp"
#include <LabJackM.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include <chrono>
#include <ctime>
#include <iomanip>

void Stream(int handle, int numChannels, const char **channelNames,
            double scanRate, int scansPerRead);

void HardcodedConfigureStream(int handle);

int main()
{
    int handle;
    double INIT_SCAN_RATE = 100;
    int SCANS_PER_READ = (int)INIT_SCAN_RATE / 2;
    enum
    {
        NUM_CHANNELS = 2
    };
    const char *CHANNEL_NAMES[] = {"AIN1", "AIN2"};

    // Open first found LabJack
    handle = OpenOrDie(LJM_dtANY, LJM_ctANY, "LJM_idANY");
    // handle = OpenSOrDie("LJM_dtANY", "LJM_ctANY", "LJM_idANY");

    PrintDeviceInfoFromHandle(handle);
    printf("\n");

    Stream(handle, NUM_CHANNELS, CHANNEL_NAMES, INIT_SCAN_RATE, SCANS_PER_READ);

    CloseOrDie(handle);
    WaitForUserIfWindows();

    return LJME_NOERROR;
}

bool interrupted = false;

void signalHandler(int signum)
{
    interrupted = true;
}

void HardcodedConfigureStream(int handle)
{
    enum
    {
        NUM_FRAMES = 6
    };
    const char *aNames[] = {"STREAM_TRIGGER_INDEX", "STREAM_CLOCK_SOURCE", "STREAM_RESOLUTION_INDEX",
                            "STREAM_SETTLING_US", "AIN_ALL_RANGE", "AIN_ALL_NEGATIVE_CH"};

    const double aValues[] = {0,
                              0,
                              4,
                              1000,
                              10,
                              LJM_GND};

    printf("Writing configurations:\n");
    WriteNamesOrDie(handle, NUM_FRAMES, aNames, aValues);
}

void Stream(int handle, int numChannels, const char **channelNames,
            double scanRate, int scansPerRead)
{
    int err, channel;
    int deviceScanBacklog = 0;
    int LJMScanBacklog = 0;
    unsigned int receiveBufferBytesSize = 0;
    unsigned int receiveBufferBytesBacklog = 0;
    int connectionType;

    int *aScanList = new int[numChannels];
    unsigned int aDataSize = numChannels * scansPerRead;
    double *aData = new double[sizeof(double) * aDataSize];

    std::ofstream file;
    file.open("test_data.csv", std::ios::out | std::ios::app);

    err = LJM_GetHandleInfo(handle, NULL, &connectionType, NULL, NULL, NULL, NULL);
    ErrorCheck(err, "LJM_GetHandleInfo");

    // Clear aData. This is not strictly necessary, but can help debugging.
    memset(aData, 0, sizeof(double) * aDataSize);

    err = LJM_NamesToAddresses(numChannels, channelNames, aScanList, NULL);
    ErrorCheck(err, "Getting positive channel addresses");

    HardcodedConfigureStream(handle);

    printf("\n");
    printf("Starting stream...\n");
    err = LJM_eStreamStart(handle, scansPerRead, numChannels, aScanList,
                           &scanRate);
    ErrorCheck(err, "LJM_eStreamStart");
    printf("Stream started. Actual scan rate: %.02f Hz (%.02f sample rate)\n",
           scanRate, scanRate * numChannels);
    printf("\n");

    signal(SIGINT, signalHandler);

    while (!interrupted)
    {
        err = LJM_eStreamRead(handle, aData, &deviceScanBacklog,
                              &LJMScanBacklog);
        ErrorCheck(err, "LJM_eStreamRead");

        // printf("iteration: %d - deviceScanBacklog: %d, LJMScanBacklog: %d",
        //        iteration, deviceScanBacklog, LJMScanBacklog);
        if (connectionType != LJM_ctUSB)
        {
            err = LJM_GetStreamTCPReceiveBufferStatus(handle,
                                                      &receiveBufferBytesSize, &receiveBufferBytesBacklog);
            ErrorCheck(err, "LJM_GetStreamTCPReceiveBufferStatus");
            printf(", receive backlog: %f%%",
                   ((double)receiveBufferBytesBacklog) / receiveBufferBytesSize * 100);
        }
        printf("\n");
        printf("  1st scan out of %d:\n", scansPerRead);

        auto now = std::chrono::system_clock::now();
        time_t rawtime = std::chrono::system_clock::to_time_t(now);
        std::tm *local_time;
        local_time = localtime(&rawtime);
        file << std::put_time(local_time, "%F %T") << ", ";

        for (channel = 0; channel < numChannels; channel++)
        {
            printf("    %s = %0.5f\n", channelNames[channel], aData[channel]);
            file << aData[channel] << ", ";
            file.flush();
        }

        file << "\n";
    }
    file << "\n";
    file.close();

    printf("Stopping stream\n");
    err = LJM_eStreamStop(handle);
    ErrorCheck(err, "Stopping stream");

    delete[] aData;
    delete[] aScanList;
}