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
//  RUN: ./myscheduler sysconfig-file command-file

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
    char name[MAX_DEVICE_NAME + 1];
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
    int execution_time;            // Time taken to execute on CPU
    int cumulative_execution_time; // Time given on command file
    char name[MAX_SYSCALL_NAME + 1];
    char spawn[MAX_COMMAND_NAME + 1]; // Command referred to when spawn is called
    struct Device device;             // Set to NULL if no device is read/written to
    int bytes;                        // Number of bytes being read/written
    int sleep_time;                   // Amount of sleep time (for sleep commands)
};

struct Command
{
    char name[MAX_COMMAND_NAME + 1];
    struct SystemCall systemCalls[MAX_SYSCALLS_PER_PROCESS];
    struct Process *spawn; // Stores pointer to parent (to tell parent when "wait" is over, where necessary)
};

struct Process
{
    struct Command command; // Stores a copy of the command and the system calls that the process is running
    int syscall_index;      // The index of the system call that the process is currently executing
    bool wait;              // Helps to keep track of when child process is done
    int event_time;         // Amount of time left until getting removed from/leaving CPU, blocked queue or databus
    int remaining_cpu_time; // Amount of time left on CPU (used if getting kicked off CPU)
};

struct Process readyqueue[MAX_RUNNING_PROCESSES];             // Stores READY processes, currently waiting to run
struct Process dataqueue[MAX_DEVICES][MAX_RUNNING_PROCESSES]; // Stores devices waiting for databus use, for each device (0 to 3)
struct Process blockedqueue[MAX_RUNNING_PROCESSES];           // Stores devices waiting for child process, or sleeping
int readyqueue_index = 0;                                     // Current index of end of readyqueue
int dataqueue_index[MAX_DEVICES] = {0};                       // Current index of end of queues of each device
int blockedqueue_index = 0;                                   // Current index of end of blockedqueue
int cputransitiontime = 0;                                    // Will be non-zero when CPU is performing state transition

// enum ProcessState
// {
//     READY,
//     RUNNING,
//     BLOCKED_IO,
//     BLOCKED_SLEEP,
//     BLOCKED_WAIT,
//     EXITED
// };

int globaltime = 0; // Global counter of time in usecs

struct Command commands[MAX_COMMANDS];

struct Event
{
    int clock_time;
    char *event_type; // Can be "data", "blocked", "cpu"
    int index;        // Index of process (related to the event) on data or blocked queue
};

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
            char value[MAX_DEVICE_NAME + 1] = {'\0'};
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

    fclose(sysconfig);

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
                        currentsyscall->cumulative_execution_time = currentsyscall->cumulative_execution_time * 10 + (token[j] - '0');
                        j++;
                    }
                    // Sets execution time to difference between current and previous cumulative execution times
                    if (syscall_index == 0)
                    {
                        currentsyscall->execution_time = currentsyscall->cumulative_execution_time;
                    }
                    else
                    {
                        currentsyscall->execution_time = currentsyscall->cumulative_execution_time - commands[command_index - 1].systemCalls[syscall_index - 1].cumulative_execution_time;
                    }
                }
                else if (word_count == 1) // Currently reading name of the system call
                {
                    sprintf(currentsyscall->name, "%s", token);
                    spawn = (strcmp(token, "spawn") == 0); // Sets spawn variable to true if this system call spawns another process
                }
                else if (word_count == 2 && isdigit(token[0]) && !spawn) // Currently reading sleep time
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
                    sprintf(currentsyscall->spawn, "%s", token);
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

bool isEmpty()
{
    return ((readyqueue_index == 0) && (blockedqueue_index == 0) && (dataqueue_index[0] == 0) && (dataqueue_index[1] == 0) && (dataqueue_index[2] == 0) && (dataqueue_index[3] == 0));
}

struct Event next_event()
{
    struct Event output;

    // Getting the minimum blocked queue time
    int blocked_event[2] = {-1, -1}; // Stores the clock value and the index of the blocked queue that is first to be released
    for (int i = 0; i < MAX_RUNNING_PROCESSES; i++)
    {
        if (strcmp(blockedqueue[i].command.name, "") == 0)
        {
            break;
        }
        else if (blocked_event[0] == -1 || blockedqueue[i].event_time < blocked_event[0])
        {
            blocked_event[0] = blockedqueue[i].event_time;
            blocked_event[1] = i;
        }
    }

    // Getting the minimum databus time
    int data_event[2] = {-1, -1}; // Stores the clock value and the index of the device that is being used (and will finish first)
    for (int i = 0; i < MAX_DEVICES; i++)
    {
        if (strcmp(dataqueue[i]->command.name, "") == 0)
        {
            continue;
        }
        else if (data_event[0] == -1 || dataqueue[i]->event_time < data_event[0])
        {
            data_event[0] = dataqueue[i]->event_time;
            data_event[1] = i;
        }
    }

    if (blocked_event[0] != -1 && (blocked_event[0] < data_event[0] || blocked_event[0] < readyqueue[0].event_time)) // Blocked is min
    {
        output.clock_time = blocked_event[0];
        output.event_type = "blocked";
        output.index = blocked_event[1];
    }
    else if (data_event[0] != -1 && (data_event[0] < readyqueue[0].event_time)) // Data is min
    {
        output.clock_time = data_event[0];
        output.event_type = "data";
        output.index = data_event[1];
    }
    else // Ready is min
    {
        output.clock_time = readyqueue[0].event_time;
        output.event_type = "cpu";
        output.index = -1;
    }
    return output;
}

void dequeue(struct Process *array, int *array_index) // Removes the first element of an array and moves remaining elements one index forward
{
    for (int i = 1; i < *array_index; i++)
    {
        array[i - 1] = array[i];
    }
    sprintf(array[*array_index - 1].command.name, "%s", ""); // Essentially "clears" the last element of the array
    --*array_index;                                          // Increments the value of the index (rather than incrementing the pointer address)
}

void fronttoback(struct Process *array, int *array_index)
{
    struct Process temp = array[0];
    dequeue(array, array_index);
    array[*array_index] = temp;
    ++*array_index; // Increments the value of the index (rather than incrementing the pointer address)
}

void execute_commands(void)
{
    // Initialises first process on command file
    readyqueue[0].command = commands[0];
    readyqueue[0].syscall_index = 0;
    readyqueue[0].event_time = fmin(timequantum, commands[0].systemCalls[0].execution_time);
    readyqueue[0].remaining_cpu_time = commands[0].systemCalls[0].execution_time - readyqueue[0].event_time;
    readyqueue_index++;
    globaltime += TIME_CONTEXT_SWITCH;

    while (!isEmpty())
    { /*
         1. increment timer to get to the next closest event (minimum time left of blocked queue, data queue and ready queue)
         2. check the returned event to see what needs to be done (for each of the following cases)
            a. cpu -> conduct appropriate system call and move new system call onto cpu
            b. blocked -> move process back to ready queue to execute next system call (and add transition time to execution time???)
            c. data -> move process back to ready queue to execute next system call (and add transition time to execution time???)
            NOTE: for each of these, if the process has the spawn field set to the parent, tell parent that the process is finished
     */
        struct Event next = next_event();
        globaltime = next.clock_time;
        if (strcmp(next.event_type, "cpu") == 0) // event is a process coming off the cpu
        {
            /*
            WHAT NEEDS TO BE DONE:
            1. Check the head of the readyqueue, if the syscall execution time was completed or the timequantum expired
            2. If timequantum expired, call fronttoback()
            3. If syscall execution time completed, view command name to see what to do (what queue to use or to exit)
            4. Calculate and occupy CPU for transition time
            */
            if (readyqueue[0].remaining_cpu_time == 0) // Process has finished executing
            {
                if (strcmp(readyqueue[0].command.systemCalls[readyqueue[0].syscall_index].name, "exit")) // If syscall is exit
                {
                    dequeue(readyqueue, &readyqueue_index);
                }
                else if (strcmp(readyqueue[0].command.systemCalls[readyqueue[0].syscall_index].name, "sleep"))
                {
                    // Do something
                }
                else if (strcmp(readyqueue[0].command.systemCalls[readyqueue[0].syscall_index].name, "wait"))
                {
                    // Do something
                }
                else if (strcmp(readyqueue[0].command.systemCalls[readyqueue[0].syscall_index].name, "spawn"))
                {
                    // Do something
                }
                else if (strcmp(readyqueue[0].command.systemCalls[readyqueue[0].syscall_index].name, "read"))
                {
                    // Do something
                }
                else if (strcmp(readyqueue[0].command.systemCalls[readyqueue[0].syscall_index].name, "write"))
                {
                    // Do something
                }
                // NEED TO INCREMENT SYSCALL INDEX!!!!!!!!!!!!!!!!!!!!!!!!!!!
            }
            else // Time quantum expired, process needs to be back on ready queue
            {
                fronttoback(readyqueue, &readyqueue_index);
                if (readyqueue[0].remaining_cpu_time == 0) // Check if syscall is on CPU for first time
                {
                    // Initialises remaining CPU time and the event time of the process
                    readyqueue[0].remaining_cpu_time = readyqueue[0].command.systemCalls[readyqueue[0].syscall_index].execution_time - fmin(readyqueue[0].command.systemCalls[readyqueue[0].syscall_index].execution_time, timequantum);
                    readyqueue[0].event_time = globaltime + fmin(readyqueue[0].command.systemCalls[readyqueue[0].syscall_index].execution_time, timequantum);
                }
                readyqueue[0].event_time = globaltime + readyqueue[0].command.systemCalls[readyqueue[0].syscall_index].execution_time;
            }
            dequeue(readyqueue, &readyqueue_index);
        }
        else if (strcmp(next.event_type, "data")) // event is a process coming off the databus
        {
        }
        else if (strcmp(next.event_type, "blocked")) // event is a process coming off the blocked queue
        {
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
    // execute_commands();

    //  PRINT THE PROGRAM'S RESULTS
    printf("measurements  %i  %i\n", 0, 0);

    exit(EXIT_SUCCESS);
}

//  vim: ts=8 sw=4

// Solution draft:

