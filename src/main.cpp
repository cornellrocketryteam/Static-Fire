/**
 * stream_data.cpp: config and stream from multiple AIN channels
 **/

#include "LJM_StreamUtilities.h"
#include <LabJackM.h>
#include <fstream>
#include <stdio.h>
#include <string.h>

void Stream(int handle, int numChannels, const char **channelNames,
            double scanRate, int scansPerRead, int numReads);

void HardcodedConfigureStream(int handle);

int main()
{
    int handle;
    double INIT_SCAN_RATE = 70;
    int SCANS_PER_READ = (int)INIT_SCAN_RATE / 2;
    const int NUM_READS = 10;
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

    Stream(handle, NUM_CHANNELS, CHANNEL_NAMES, INIT_SCAN_RATE, SCANS_PER_READ,
           NUM_READS);

    CloseOrDie(handle);
    WaitForUserIfWindows();

    return LJME_NOERROR;
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
            double scanRate, int scansPerRead, int numReads)
{
    int err, iteration, channel;
    int deviceScanBacklog = 0;
    int LJMScanBacklog = 0;
    unsigned int receiveBufferBytesSize = 0;
    unsigned int receiveBufferBytesBacklog = 0;
    int connectionType;

    int *aScanList = new int[numChannels];
    unsigned int aDataSize = numChannels * scansPerRead;
    double *aData = new double[sizeof(double) * aDataSize];

    std::ofstream file("test_data.csv");

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

    // Read the scans
    printf("Now performing %d reads\n", numReads);
    printf("\n");
    // change to while + delete numReads and NUM_READS
    for (iteration = 0; iteration < numReads; iteration++)
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
        for (channel = 0; channel < numChannels; channel++)
        {
            printf("    %s = %0.5f\n", channelNames[channel], aData[channel]);
            file << aData[channel] << ", ";
        }

        file << "\n";
    }

    file.close();

    printf("Stopping stream\n");
    err = LJM_eStreamStop(handle);
    ErrorCheck(err, "Stopping stream");

    delete[] aData;
    delete[] aScanList;
}