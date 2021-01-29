/*
Copyright (c) 2016-2018 Granite Devices Oy

This file is made available under the terms of
Granite Devices Software End-User License Agreement, available at
https://granitedevices.com/legal

Contributions and modifications are allowed only under the terms of
Granite Devices Contributor License Agreement, available at
https://granitedevices.com/legal

3rd-party contributors:


*/
#include <stdio.h>
#include <wchar.h>

#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <strings.h>
#include <unistd.h>
#include <windows.h>

#include "hidapi.h"
#include "config_comm_defines.h"

hid_device *simucubehandle;
#define gdusbvid 0x16d0

// Simucube 1:
#define simucube1pid 0x0d5a
// Simucube 2 Sport:
#define simucube2Spid 0x0d61
// Simucube 2 Pro:
#define simucube2Ppid 0x0d60
// Simucube 2 Ultimate:
#define simucube2Upid 0x0d5f

const unsigned short PIDS[] = {simucube1pid, simucube2Spid, simucube2Ppid, simucube2Upid};
const int PIDS_SIZE = sizeof(PIDS) / sizeof(PIDS[0]);

bool connectSimuCube() {
    hid_exit();
    hid_init();
    for (int i = 0; i < PIDS_SIZE; ++i) {
        simucubehandle = hid_open(gdusbvid, PIDS[i], NULL);
        if (simucubehandle)
            return true;
    }
    return false;
}


bool writeSimucube(uint8_t *data) {
    hid_set_nonblocking(simucubehandle, 1);

    // note: must always write 60 bytes (the size set in SimuCUBE
    // HID Descriptor). This is because some Windows versions
    // will discard / not transmit if size does not match descriptor.
    if(hid_write(simucubehandle, data, 60) == -1) {
        // error
        return false;
    }
    return true;
}



// NOTE:
// Also this had to be added in the QT project file:
// LIBS += -lSetupAPI


int main()
{
    printf("Hello World!\r\n");
    commandPacket commandData;
    // software could transmit just the commanddata packet,
    // but as will have to trasmit 60 bytes, it is possible
    // that would overrun the program memory limits and crash.
    // therefore it is more safe to create this type of buffer
    // that is, for sure, large enough for any transmissions.
    unsigned char transmitbuf[256];
    memset(transmitbuf,0x00,sizeof(transmitbuf));
    bool connect = connectSimuCube();


    // set steering mode. Call this every time user goes into car
    // and also when the game main window is reactivated, if user
    // is in car driving.

    uint16_t lockToLockDegrees = 540; // this can be parametrized :)
    if(connect) {
        commandData.reportID = outReport;
        commandData.command = setTemporaryVariable;
        commandData.value = temporarySteeringAngle;
        commandData.value2 = lockToLockDegrees;

        memcpy(&transmitbuf, &commandData, sizeof(commandPacket));

        if(!writeSimucube(transmitbuf)) {
               hid_close(simucubehandle);
               simucubehandle = NULL;
               printf("Command failed.\n");
               return 0;
        }
        //success
        printf("Command sent successfully. Wheel set to: %d degrees.\n", lockToLockDegrees);
    }

    int time = 10;
    printf("The wheel rotation will be reverted in %d seconds.\n", time);
    while(time--) {
        printf(".");
        sleep(1);
    }
    printf("\n");

    // Unset the game-settable steering angle.
    // Call this at least on game exit.
    if(connect) {
        commandData.reportID = outReport;
        commandData.command = setTemporaryVariable;
        commandData.value=unsetTemporarySteeringAngle;
        commandData.value2 = lockToLockDegrees;

        memcpy(&transmitbuf, &commandData, sizeof(commandPacket));
        if(!writeSimucube(transmitbuf)) {
           hid_close(simucubehandle);
           simucubehandle = NULL;
           printf("Command failed.\n");
           return 0;
        }
        //success
        printf("Command sent successfully. Wheel rotation reverted.\n");
    }

    hid_close(simucubehandle);
    return 0;
}
