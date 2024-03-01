# Static-Fire

Assists in static fire testing of our solid motor.

## Overview

To support static fire tests, this project streams data from the LabJack T7 and logs it to a CSV file. In the current version, it streams from two analog pins to collect load cell and pressure transducer data. The code in ```main.cpp``` adapts example code from LabJack (https://github.com/labjack/C_CPP_LJM). The files in ```lib/``` are modified from the header files provided by the LJM library mentioned below. When running this project, use LJM headers from ```lib/``` instead of the LJM library headers ```LJM_StreamUtilities.h``` and ```LJM_Utilities.h```.

## Getting Started
### Dependencies Required
* ```cmake```
* ```LJM library``` (https://labjack.com/pages/support?doc=/software-driver/installer-downloads/ljm-software-installers-t4-t7-digit/)

## Running
1. Create a top-level ```build/``` directory
2. Run ```cmake ..``` from within ```build/```
3. Run ```make```
4. Run ```./solid```
