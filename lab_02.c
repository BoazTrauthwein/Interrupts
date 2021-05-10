#include <stdio.h>
#include <dos.h>
#include <time.h>

typedef int bool;

#define true 1
#define false 0
#define SYSTEM_SPEED 16

int endProgram;
int pulse_time = 0;
int timerToExit = -1;
int speed = 16;
int counter;
int totalTime;
unsigned int right_flag_shift, left_flag_shift;

void interrupt (*Int8Save)(void); // Pointer to interrupt function
void interrupt (*Int9Save)(void);
void endProgramFunc();
void HandleScanCode(unsigned char scan);

void interrupt My_Int8_Handler(void)
{
    pulse_time++;
    totalTime++;
    if (pulse_time > (int)(5 * 18.2)) //Print speed every 5 seconds.
    {
        printf("5 sec : speed : 1 / %d\n", SYSTEM_SPEED / speed);
        pulse_time = 0; //	reset timer to start a new 5 seconds count
    }

    if (totalTime % (SYSTEM_SPEED / speed) == 0) // if this is the interrupt timed to work normally
    {                                            // and wake up the awaiting ones in a loop with raising the counter 'flag'
        Int8Save();
        counter = 1;
    }
    else // meaning we need to slow down system by raising interrupt flag, allowing a new interrupt 8 to occur and putting
    {    // this segment in a loop until the correct time interrupt is engaged based on modulo with the system's speed
        Int8Save();
        asm {
			STI
        }
        counter = 0;
        while (counter == 0)
            ;
    }
    if (timerToExit > -1)
    {                               //	If the two shifts are pressed.
        if (timerToExit > 3 * 18.2) //	exit the program if the two shifts have been pressed together for over 3 seconds continuously
            endProgramFunc();
        timerToExit++;
    }
}

void interrupt h9(void)
{
    unsigned char scan;
    asm {
		in 	al,60h //	place scan code of the key pressed in al
		mov scan, al //	save the scan code in a variable to check later
    }
    HandleScanCode(scan);
    if (right_flag_shift == true && left_flag_shift == true && timerToExit == -1) // if the two shifts are pressed together, reset timerToExit to begin counting time
        timerToExit = 0;
    Int9Save();
}

void HandleScanCode(unsigned char scan)
{
    switch (scan)
    {
    case 42: //	right shift is pressed
    {
        right_flag_shift = true;
        break;
    }
    case 54: //	left shift is pressed
    {
        left_flag_shift = true;
        break;
    }
    case 170: //	right shift was released
    {
        right_flag_shift = false;
        timerToExit = -1; //	if one of the shift keys was released, reset timerToExit so we'll stop counting the time to exit program
        break;
    }
    case 182: //	left shift was released
    {
        left_flag_shift = false;
        timerToExit = -1;
        break;
    }
    case 72: //	arrow up is pressed
    {
        if (speed < SYSTEM_SPEED) // checks if this is the max speed of 1/1
        {
            speed *= 2;
            printf("speed : 1 / %d\n", SYSTEM_SPEED / speed);
        }
        break;
    }
    case 80: // arrow down is pressed
    {
        if (speed > 1) // checks if this is the minimum speed of 1/16
        {
            speed /= 2;
            printf("speed : 1 / %d\n", SYSTEM_SPEED / speed);
        }
        break;
    }
    default:
        break;
    }
}

void slowPrg()
{
    endProgram = 0;
    Int8Save = getvect(8);       // Preserve old pointer
    setvect(8, My_Int8_Handler); // Set entry to new handler
    Int9Save = getvect(9);
    setvect(9, h9);
}

void endProgramFunc()
{
    printf("finish");
    setvect(8, Int8Save); // Restore old pointers
    setvect(9, Int9Save);
    exit(0);
}

int main()
{
    unsigned long int i, j;
    long long int counter = 0;
    time_t t1, t2;
    i = j = 0;
    slowPrg();
    while (1)
    {
        (void)time(&t1);
        j = 0;
        while (j < 10)
        {
            i++;
            counter++;
            if ((i % 10000) == 0)
            {
                printf("counter = %lld\n", counter);
                j++;
            }
        } // while
        (void)time(&t2);
        printf("\nTotal  Iteration  Time -  = %d secondsn\n", (int)(t2 - t1));
    }
}
