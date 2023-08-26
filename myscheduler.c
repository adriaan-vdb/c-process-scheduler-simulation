#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
//  you may need other standard header files

//  CITS2002 Project 1 2023
//  Student1:   23336556   Adriaan van der Berg
//  Student2:   23432725   Joshua Then

//  myscheduler (v1.0)
//  Compile with:  cc -std=c11 -Wall -Werror -o myscheduler myscheduler.c

//  THESE CONSTANTS DEFINE THE MAXIMUM SIZE OF sysconfig AND command DETAILS
//  THAT YOUR PROGRAM NEEDS TO SUPPORT.  YOU'LL REQUIRE THESE //  CONSTANTS
//  WHEN DEFINING THE MAXIMUM SIZES OF ANY REQUIRED DATA STRUCTURES.

#define MAX_DEVICES 4
#define MAX_DEVICE_NAME 20
#define MAX_COMMANDS 10
#define MAX_COMMAND_NAME 20
#define MAX_SYSCALLS_PER_PROCESS 40
#define MAX_RUNNING_PROCESSES 50

//  NOTE THAT DEVICE DATA-TRANSFER-RATES ARE MEASURED IN BYTES/SECOND,
//  THAT ALL TIMES ARE MEASURED IN MICROSECONDS (usecs),
//  AND THAT THE TOTAL-PROCESS-COMPLETION-TIME WILL NOT EXCEED 2000 SECONDS
//  (SO YOU CAN SAFELY USE 'STANDARD' 32-BIT ints TO STORE TIMES).

#define DEFAULT_TIME_QUANTUM 100

#define TIME_CONTEXT_SWITCH 5
#define TIME_CORE_STATE_TRANSITIONS 10
#define TIME_ACQUIRE_BUS 20

//  ----------------------------------------------------------------------
int timequantum = DEFAULT_TIME_QUANTUM;

enum ProcessState
{
    READY,
    RUNNING,
    BLOCKED_IO,
    BLOCKED_SLEEP,
    BLOCKED_WAIT,
    EXITED
};

struct Process
{
    int pid;                  // Process ID
    enum ProcessState state;  // Current state of the process
    int remainingTimeQuantum; // Remaining time quantum if in RUNNING state
    int elapsedTime;          // Total elapsed time in microseconds
    // Add more attributes as needed, e.g., IO device info, sleep duration, etc.
    // ...
    struct Process *next; // Pointer to the next process in the queue
};

struct Device
{
    char name[20];
    int readspeed;
    int writespeed;
    struct Process queue[MAX_RUNNING_PROCESSES];
};

struct Device devices[MAX_DEVICES];

void read_sysconfig(char argv0[], char filename[])
{
    struct Device tempdevices[MAX_DEVICES]; // Temporarily stores unordered devices, loaded in from sysconfig
    FILE *sysconfig = fopen(filename, "r");
    // Check if file failed to read
    if (sysconfig == NULL)
    {
        exit(EXIT_FAILURE);
    }
    int device = 0;
    char buffer[256];
    while (fgets(buffer, 256, sysconfig))
    {
        // Check if reading a comment
        if ((buffer[0] == '#'))
        {
            if (device > 0)
            {
                device = MAX_DEVICES; // Finished recording devices, start recording timequantum value
            }
            continue;
        }
        if (device < MAX_DEVICES)
        {
            int i = 6;                                    // Keeps track of what character is being inspected
            for (int feature = 0; feature < 3; feature++) // Keeps track of what is being recorded (0 = name, 1 = readspeed, 2 = writespeed)
            {
                char value[20] = {'\0'};
                while (buffer[i] == '\t' || buffer[i] == ' ')
                {
                    i++;
                }
                while (buffer[i] != '\t' && buffer[i] != ' ')
                {
                    if (feature != 0 && buffer[i] == 'B') // Terminate character recording before "Bps" - so string is a number
                    {
                        i += 3; // Set character index to after "Bps"
                        break;
                    }
                    sprintf(value, "%s%c", value, buffer[i]);
                    i++;
                }
                switch (feature)
                {
                case 0:
                    sprintf(tempdevices[device].name, "%s", value);
                case 1:
                    tempdevices[device].readspeed = atoi(value);
                case 2:
                    tempdevices[device].writespeed = atoi(value);
                }
            }
            device++;
        }
        else
        {
            char value[20] = {'\0'};
            int i = 11;
            while (buffer[i] == '\t' || buffer[i] == ' ')
            {
                i++;
            }
            while (buffer[i] != '\t' && buffer[i] != ' ' && i < 256)
            {
                if (buffer[i] == 'u')
                {
                    break;
                }
                sprintf(value, "%s%c", value, buffer[i]);
                i++;
            }
            timequantum = atoi(value);
        }
        buffer[0] = '\0';
    }
    // Sort devices in ascending order of read speed
    int minreadspeed = 0; // Keeps track of index of device with current minimum read speed
    for (int i = 0; i < 4; i++)
    {
        minreadspeed = 0;
        for (int j = 0; j < 4; j++)
        {
            while (tempdevices[minreadspeed].readspeed == -1)
            {
                minreadspeed++;
                j++;
            }
            if (tempdevices[j].readspeed < tempdevices[minreadspeed].readspeed && tempdevices[j].readspeed != -1)
            {
                minreadspeed = j;
            }
        }
        devices[i] = tempdevices[minreadspeed];
        tempdevices[minreadspeed].readspeed = -1;
    }

    // for (int i = 0; i < 4; i++)
    // {
    //     printf("%s ", devices[i].name);
    //     printf("%d ", devices[i].readspeed);
    //     printf("%d \n", devices[i].writespeed);
    // }
    // printf("%d", timequantum);
}

void read_commands(char argv0[], char filename[])
{
}

//  ----------------------------------------------------------------------

void execute_commands(void)
{
}

//  ----------------------------------------------------------------------

int main(int argc, char *argv[])
{
    //  ENSURE THAT WE HAVE THE CORRECT NUMBER OF COMMAND-LINE ARGUMENTS
    if (argc != 3)
    {
        printf("Usage: %s sysconfig-file command-file\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    //  READ THE SYSTEM CONFIGURATION FILE
    read_sysconfig(argv[0], argv[1]);

    //  READ THE COMMAND FILE
    read_commands(argv[0], argv[2]);

    //  EXECUTE COMMANDS, STARTING AT FIRST IN command-file, UNTIL NONE REMAIN
    execute_commands();

    //  PRINT THE PROGRAM'S RESULTS
    printf("measurements  %i  %i\n", 0, 0);

    exit(EXIT_SUCCESS);
}

//  vim: ts=8 sw=4

// Solution draft:
