// Example of how to structure packet data:



// Define the states a process can be in
enum ProcessState {
    READY,
    RUNNING,
    BLOCKED_IO,
    BLOCKED_SLEEP,
    BLOCKED_WAIT,
    EXITED
};

// Define a data structure for processes
struct Process {
    int pid;                   // Process ID
    enum ProcessState state;   // Current state of the process
    int remainingTimeQuantum;  // Remaining time quantum if in RUNNING state
    int elapsedTime;           // Total elapsed time in microseconds
    // Add more attributes as needed, e.g., IO device info, sleep duration, etc.
    // ...
    //struct Process *next;      // Pointer to the next process in the queue
};

// Define a data structure for the sysconfig
struct Device {
    char devicename[20];
    int readspeed;
    int writespeed;
};

struct SysConfig {
    struct Device devices[4];  // Array of I/O devices
    int timequantum;           // Time quantum for scheduling
};

// Define a data structure for commands
struct SystemCall {
    int elapsed_time;  // Elapsed time in microseconds
    // Add more system call attributes as needed
    // ...
};

struct Command {
    char commandName[20];
    struct SystemCall syscalls[/*max number of syscalls*/];
};

// Define functions for process manipulation, scheduling, etc.
void initializeProcess(struct Process *process, int pid, enum ProcessState state);
void addToQueue(struct Process **queue, struct Process *process);
struct Process *popFromQueue(struct Process **queue);
// Add more functions as needed

int main() {
    // Read sysconfig file and populate sysconfig structure
    // Read command file and populate command structures

    // Initialize queues for each process state
    struct Process *readyQueue = NULL;
    struct Process *runningProcess = NULL;
    struct Process *blockedIOQueue = NULL;
    struct Process *blockedSleepQueue = NULL;
    struct Process *blockedWaitQueue = NULL;
    struct Process *exitedQueue = NULL;

    // Simulation loop
    int currentTime = 0;
    while (/*simulation not finished*/) {
        // Handle process state transitions, time quantums, I/O, etc.

        // Move processes between queues based on their state and elapsed time

        // Schedule the next process to run based on queue priorities, time quantums, etc.

        // Simulate the execution of the selected process

        // Update time and process attributes accordingly
    }

    // Cleanup and exit
    return 0;
}