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

struct SystemCall
{
    // int elapsed_time; // Elapsed time in microseconds
    int execution_time; // Time taken to execute on CPU
    char name[MAX_SYSCALL_NAME];
    char spawn[MAX_COMMAND_NAME]; // Stores name of command being spawned (where relevant)
    int bytes;                    // Number of bytes being read/written
    int sleep_time;               // Amount of sleep time (for sleep commands)
};

struct Command
{
    char name[MAX_COMMAND_NAME];
    struct SystemCall systemCalls[MAX_SYSCALLS_PER_PROCESS];
};

int main()
{
    struct Command test;

    // Assign a value to the name field within inner
    strcpy(test.systemCalls[0].name, "Hello, World!");

    // Print the value
    printf("Name: %s\n", test.systemCalls[0].name);

    return 0;
}