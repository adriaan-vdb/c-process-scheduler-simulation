#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
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

#define MAX_LINE_LENGTH 256
#define MAX_SYSCALL_NAME 5

//  NOTE THAT DEVICE DATA-TRANSFER-RATES ARE MEASURED IN BYTES/SECOND,
//  THAT ALL TIMES ARE MEASURED IN MICROSECONDS (usecs),
//  AND THAT THE TOTAL-PROCESS-COMPLETION-TIME WILL NOT EXCEED 2000 SECONDS
//  (SO YOU CAN SAFELY USE 'STANDARD' 32-BIT ints TO STORE TIMES).

#define DEFAULT_TIME_QUANTUM 100

#define TIME_CONTEXT_SWITCH 5
#define TIME_CORE_STATE_TRANSITIONS 10
#define TIME_ACQUIRE_BUS 20

#define MAX_LINE_LENGTH 256

//  ----------------------------------------------------------------------
char delimiters[] = " \t"; // Watches out for both space and tab characters

int timequantum = DEFAULT_TIME_QUANTUM;

struct Device
{
    char name[MAX_DEVICE_NAME];
    int readspeed;
    int writespeed;
};

struct Device devices[MAX_DEVICES];

// enum SystemCallType
// {
//     SPAWN,
//     READ,
//     WRITE,
//     SLEEP,
//     WAIT,
//     EXIT,
// };

struct SystemCall
{
    // int elapsed_time; // Elapsed time in microseconds
    int execution_time; // Time taken to execute on CPU
    char name[MAX_SYSCALL_NAME];
    char spawn[MAX_COMMAND_NAME]; // Stores name of command being spawned (where relevant)
    struct Device device;         // Set to NULL if no device is read/written to
    int bytes;                    // Number of bytes being read/written
    int sleep_time;               // Amount of sleep time (for sleep commands)
};

struct Command
{
    char name[MAX_COMMAND_NAME];
    struct SystemCall systemCalls[MAX_SYSCALLS_PER_PROCESS];
};

struct Command blockedqueue[MAX_RUNNING_PROCESSES];           // Stores READY processes, currently waiting to run
struct Command dataqueue[MAX_DEVICES][MAX_RUNNING_PROCESSES]; // Stores devices waiting for databus use, for each device (0 to 3)
int blockedqueue_index = 0;
int dataqueue_index[MAX_DEVICES] = {0};

// enum ProcessState
// {
//     READY,
//     RUNNING,
//     BLOCKED_IO,
//     BLOCKED_SLEEP,
//     BLOCKED_WAIT,
//     EXITED
// };

struct Process
{
    struct Command command;    // The command that the process is running
    int syscall_index;         // The index of the system call that the process is currently executing
    int remainingsyscall_time; // Remaining time of the current system call
};

int globaltime = 0; // Global counter of time in usecs

struct Command commands[MAX_COMMANDS];

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
    char buffer[MAX_LINE_LENGTH];
    while (fgets(buffer, MAX_LINE_LENGTH, sysconfig))
    {
        // Check if reading a comment
        if (buffer[0] == '#')
        {
            if (device > 0)
            {
                device = MAX_DEVICES; // Finished recording devices, start recording timequantum value
            }
            continue;
        }
        if (device < MAX_DEVICES)
        {
            char *token = strtok(buffer, delimiters); // skips "device" name
            // int i = strlen("device");                     // Sets character index to whitespace after "device" title
            for (int feature = 0; feature < 3; feature++) // Keeps track of what is being recorded (0 = name, 1 = readspeed, 2 = writespeed)
            {
                token = strtok(NULL, delimiters);
                int value = 0;
                int j = 0;
                switch (feature)
                {
                case 0:
                    sprintf(tempdevices[device].name, "%s", token);
                case 1:

                    while (isdigit(token[j]))
                    {
                        value = value * 10 + (token[j] - '0');
                        j++;
                    }
                    tempdevices[device].readspeed = value;
                case 2:
                    while (isdigit(token[j]))
                    {
                        value = value * 10 + (token[j] - '0');
                        j++;
                    }
                    tempdevices[device].writespeed = value;
                }
            }
            device++;
        }
        else
        {
            char value[MAX_DEVICE_NAME] = {'\0'};
            int i = strlen("timequantum"); // Sets character index to whitespace after "timequantum" title
            while (buffer[i] == '\t' || buffer[i] == ' ')
            {
                i++;
            }
            while (buffer[i] != '\t' && buffer[i] != ' ' && i < MAX_LINE_LENGTH)
            {
                if (buffer[i] == 'u') // Terminates character recording before "usec" - so string is a number
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

// int stripDigit(char *input_string)
// {
//     int digits = 0;
//     for (int i = 0; input_string[i] != '\0'; i++)
//     {
//         if (isdigit(input_string[i]))
//         {
//             digits = digits * 10 + (input_string[i] - '0');
//         }
//     }
//     return digits;
// }

void read_commands(char argv0[], char filename[])
{
    FILE *commandfile = fopen(filename, "r");
    // Check if file failed to read
    if (commandfile == NULL)
    {
        exit(EXIT_FAILURE);
    }

    char lines[100][MAX_LINE_LENGTH];
    int line_count = 0;

    char line[MAX_LINE_LENGTH];

    // Reads entire file to lines[][] array
    while (fgets(line, sizeof(line), commandfile))
    {
        line[strcspn(line, "\n")] = '\0'; // Replaces new line character at end of line with string terminator character
        strcpy(lines[line_count], line);
        line_count++;
    }

    int command_index = 0;
    int syscall_index = 0;
    for (int i = 0; i < line_count; i++)
    {
        sprintf(line, "%s", lines[i]);
        if (strcmp(line, "#") == 0) // Checks if we are reading a # line
        {
            continue;
        }
        else if (line[0] != ' ' && line[0] != '\t') // Checks that we are reading command line (not system call)
        {
            sprintf(commands[command_index].name, "%s", line);
            syscall_index = 0; // Sets the index of the system calls array to 0 (to start recording system calls)
            command_index++;
        }
        else // Reads the system call line and appropriately records in system call struct
        {
            struct SystemCall *currentsyscall = &commands[command_index - 1].systemCalls[syscall_index]; // Current system call being recorded;
            char *token = strtok(line, delimiters);
            int word_count = 0;
            bool spawn = false; // Records if the current system calls/spawns another command
            while (token != NULL)
            {
                if (word_count == 0) // Currently reading execution_time of the system call
                {
                    int j = 0;                // Index of current character (digit) being recorded
                    while (isdigit(token[j])) // Records numbers (and stops recording before "usecs")
                    {
                        currentsyscall->execution_time = currentsyscall->execution_time * 10 + (token[j] - '0');
                        j++;
                    }
                }
                else if (word_count == 1) // Currently reading name of the system call
                {
                    sprintf(currentsyscall->name, "%s", token);
                    spawn = (strcmp(token, "spawn") == 0); // Sets spawn variable to true if this system call spawns another process
                }
                else if (word_count == 2 && isdigit(token[0])) // Currently reading sleep time
                {
                    int j = 0;                // Index of current character (digit) being recorded
                    while (isdigit(token[j])) // Records numbers (and stops recording before "usecs")
                    {
                        currentsyscall->sleep_time = currentsyscall->sleep_time * 10 + (token[j] - '0');
                        j++;
                    }
                }
                else if (word_count == 2 && !isdigit(token[0]) && spawn) // Currently reading name of new process being spawned
                {
                    for (int j = 0; j < MAX_COMMANDS; j++)
                    {
                        sprintf(currentsyscall->spawn, "%s", token);
                    }
                }
                else if (word_count == 2 && !isdigit(token[0] && !spawn)) // Currently reading name of device being read/written to
                {
                    for (int j = 0; j < MAX_DEVICES; j++)
                    {
                        if (strcmp(devices[j].name, token) == 0)
                        {
                            currentsyscall->device = devices[j];
                            break;
                        }
                    }
                    token = strtok(NULL, delimiters); // Read number of bytes to be written/read
                    int j = 0;                        // Index of current character (digit) being recorded
                    while (isdigit(token[j]))         // Records numbers (and stops recording before "B")
                    {
                        currentsyscall->bytes = currentsyscall->bytes * 10 + (token[j] - '0');
                        j++;
                    }
                }
                token = strtok(NULL, delimiters);
                word_count++;
            }
            syscall_index++;
        }
    }
    fclose(commandfile);
}

//  ----------------------------------------------------------------------

void execute_commands(void)
{
    // Initialises process and commands by adding them to the appropriate queue
    for (int i = 0; i < MAX_COMMANDS && commands[i].name != NULL; i++)
    {
        struct SystemCall current_syscall = commands[i].systemCalls[0];
        if (strcmp(current_syscall.name, "read") == 0 || strcmp(current_syscall.name, "write") == 0) // Process needs to be on dataqueue
        {
            int j = 0;
            while (j < MAX_DEVICES && devices[j].name != current_syscall.device.name)
            { // Finds out the index (j) of the device
                j++;
            }
            dataqueue[j][dataqueue_index[j]] = commands[i];
            dataqueue_index[j]++;
        }
        else // Process needs to be on blockqueue
        {
            blockedqueue[blockedqueue_index] = commands[i];
            blockedqueue_index++;
        }
    }
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

    // READ THE SYSTEM CONFIGURATION FILE
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
