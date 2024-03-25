/**
 * main.cpp: Streams data from the LabJack T7 and logs to a CSV file.
 **/

#include "LJMStreamUtil.hpp"
#include "LJMUtil.hpp"
#include <LabJackM.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <thread>
#include <chrono>
// PT: AIN1, LC: +AIN2, -AIN3

/* Constants */
int handle;
double INIT_SCAN_RATE = 100;
int SCANS_PER_READ = (int)INIT_SCAN_RATE / 2;
enum
{
    NUM_CHANNELS = 2
};
const char *CHANNEL_NAMES[] = {"AIN1", "AIN2"};
enum
{
    NUM_FRAMES = 8
};
const char *aNames[] = {"STREAM_TRIGGER_INDEX", "STREAM_CLOCK_SOURCE", "STREAM_RESOLUTION_INDEX", "STREAM_SETTLING_US",
                        "AIN_ALL_RANGE", "AIN_ALL_NEGATIVE_CH",
                        "AIN2_RANGE", "AIN2_NEGATIVE_CH"};
const double aValues[] = {0, 0, 4, 1000,
                          10, LJM_GND,
                          0.1, 3};

/* Signal handler for program quitting */
bool interrupted = false;
bool running = true;

void signalHandler(int signum)
{
    interrupted = true;
}

/* Stream data from the LabJack and store result in aData */
void Stream(int handle, double *&aData, bool &running)
{
    int err;
    int deviceScanBacklog = 0;
    int LJMScanBacklog = 0;
    unsigned int receiveBufferBytesSize = 0;
    unsigned int receiveBufferBytesBacklog = 0;
    int connectionType;
    int *aScanList = new int[NUM_CHANNELS];

    err = LJM_GetHandleInfo(handle, NULL, &connectionType, NULL, NULL, NULL, NULL);
    ErrorCheck(err, "LJM_GetHandleInfo");

    err = LJM_NamesToAddresses(NUM_CHANNELS, CHANNEL_NAMES, aScanList, NULL);
    ErrorCheck(err, "Getting positive channel addresses");

    printf("Writing configurations:\n");
    WriteNamesOrDie(handle, NUM_FRAMES, aNames, aValues);

    printf("\n");
    printf("Starting stream...\n");
    err = LJM_eStreamStart(handle, SCANS_PER_READ, NUM_CHANNELS, aScanList,
                           &INIT_SCAN_RATE);
    ErrorCheck(err, "LJM_eStreamStart");
    printf("Stream started. Actual scan rate: %.02f Hz (%.02f sample rate)\n",
           INIT_SCAN_RATE, INIT_SCAN_RATE * NUM_CHANNELS);
    printf("\n");

    signal(SIGINT, signalHandler);

    while (running)
    {
        err = LJM_eStreamRead(handle, aData, &deviceScanBacklog,
                              &LJMScanBacklog);
        ErrorCheck(err, "LJM_eStreamRead");

        if (connectionType != LJM_ctUSB)
        {
            err = LJM_GetStreamTCPReceiveBufferStatus(handle,
                                                      &receiveBufferBytesSize, &receiveBufferBytesBacklog);
            ErrorCheck(err, "LJM_GetStreamTCPReceiveBufferStatus");
            printf(", receive backlog: %f%%",
                   ((double)receiveBufferBytesBacklog) / receiveBufferBytesSize * 100);
        }
        printf("\n");
        printf("  1st scan out of %d:\n", SCANS_PER_READ);

        running = !interrupted;
    }

    printf("Stopping stream\n");
    err = LJM_eStreamStop(handle);
    ErrorCheck(err, "Stopping stream");
    delete[] aScanList;
}

void log(double *&aData, bool &running)
{
    std::ofstream file;
    file.open("test_data.csv", std::ios::out | std::ios::app);
    auto startTime = std::chrono::steady_clock::now();

    while (running)
    {
        auto currentTime = std::chrono::steady_clock::now() - startTime;
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime).count();
        file << millis << ", ";

        for (int channel = 0; channel < NUM_CHANNELS; channel++)
        {
            printf("    %s = %0.5f\n", CHANNEL_NAMES[channel], aData[channel]);
            file << aData[channel] << ", ";
            file.flush();
        }

        file << "\n";
        std::this_thread::sleep_for(std::chrono::microseconds(1000));
    }

    file << "\n";
    file.close();
}

int main()
{

    // Open first found LabJack
    handle = OpenOrDie(LJM_dtANY, LJM_ctANY, "LJM_idANY");

    PrintDeviceInfoFromHandle(handle);
    printf("\n");

    unsigned int aDataSize = NUM_CHANNELS * SCANS_PER_READ;
    double *aData = new double[sizeof(double) * aDataSize];
    memset(aData, 0, sizeof(double) * aDataSize);

    // Stream(handle, aData, running);
    // log(aData, running);

    std::thread read(Stream, handle, std::ref(aData), std::ref(running));
    std::thread write(log, std::ref(aData), std::ref(running));

    read.join();
    write.join();

    delete[] aData;
    CloseOrDie(handle);
    WaitForUserIfWindows();

    return LJME_NOERROR;
}