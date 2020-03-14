#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <iostream>
#include "826api.h"
#include <fstream>
#include <iostream>

#define DIO(C)                  ((uint64)1 << (C))                          // convert dio channel number to uint64 bit mask
#define DIOMASK(N)              {(uint)(N) & 0xFFFFFF, (uint)((N) >> 24)}   // convert uint64 bit mask to uint[2] array
#define DIOSTATE(STATES,CHAN)   ((STATES[CHAN / 24] >> (CHAN % 24)) & 1)    // extract dio channel's boolean state from uint[2] array

int galvo_offset(int* initX, int* initY)
{

    S826_SystemOpen();

    cout << "galvo calibration start" << endl;
    S826_DacRangeWrite(0, 0, S826_DAC_SPAN_10_10, 0);
    S826_DacRangeWrite(0, 1, S826_DAC_SPAN_10_10, 0);


    while (true)
    {

        SHORT leftKeyState = GetAsyncKeyState(VK_LEFT);
        SHORT rightKeyState = GetAsyncKeyState(VK_RIGHT);
        SHORT upKeyState = GetAsyncKeyState(VK_UP);
        SHORT downKeyState = GetAsyncKeyState(VK_DOWN);
        SHORT escKeyState = GetAsyncKeyState(VK_ESCAPE);

        if ((1 << 15) & leftKeyState)
            *initX -=5;




        if ((1 << 15) & rightKeyState)
            *initX += 5;



        if ((1 << 15) & upKeyState)
            *initY += 5;



        if ((1 << 15) & downKeyState)
            *initY -= 5;

        if (escKeyState)
            break;

        // 0->y, 1->x 38000    0--10v  65535 -10v    0-65535/2

        // 0.5 v per degree   -> 0.05 per 0.1 degree
        S826_DacDataWrite(0, 0, *initY, 0);
        S826_DacDataWrite(0, 1, *initX, 0);

        printf("%c %d %c %d\r", 'x', *initX, 'y', *initY);
        Sleep(100);
    }


    S826_SystemClose();
    return 0;
}

void galvo_signal(int initX, int initY)
{
    S826_DacDataWrite(0, 0, initY, 0);
    S826_DacDataWrite(0, 1, initX, 0);
}