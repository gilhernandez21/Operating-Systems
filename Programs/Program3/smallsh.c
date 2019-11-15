#define _GNU_SOURCE

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SMALLSH_MAX_ARGS 512
#define SMALLSH_MAX_CHAR 2048

/* PROTOTYPES */
// Display Prompt
void writePrompt();
// Get and Translate User Input
void getInput(char** inputPtr, size_t* bufferPtr);
void tokenizeInput(char* input, char** arguments);
// Check and Process Commands
// int checkInput(char* input, char** arguments, int* exitStatus, pid_t** backPIDs, int* numPIDs);
void replacePID(char* input);
// Redirection Functions
int _findString(char** array, char* string);
int checkRedirectOut(char** arguments);
int checkRedirectIn(char** arguments);
void redirectInSetup(int index, int* source, char** arguments);
void redirectOutSetup(int index, int* target, char** arguments);
void redirectSetupBG(int rInIndex, int rOutIndex, int* source, int* target);
// Execution Functions
int executeCmd(char** arguments);
int smallsh_exec(char** arguments, pid_t** backPIDs, int* numPIDs, int* exitStatus, struct sigaction sigact, int* termNormal);
// Background/Foreground Functions
int isBackground(char** arguments);
void savePID(pid_t** array, int* size, pid_t input);
// Built-In Functions
void smallsh_cd(char** arguments);
void smallsh_status(int terminatedNormal, int exitStatus);
void smallsh_exit(char* input, pid_t* backPIDs, int numPIDS);
// Cleanup Functions
void resetArguments(char** arguments);
void cleanInput(char** inputPtr);
void waitChildren(pid_t* array, int size);
// Signal Action
void catchSIGTSTP(int signo);

/* GLOBAL VARIABLES */
int backgroundEnabled = 1;          // Flag for if background commands are enabled

int main()
{
    int exitStatus = 0;                 // The Exit Value of the Last Foreground Command
    int terminatedNormally = 1;         // Flag for if the last foreground command terminated normally
    // int backgroundEnabled = 1;          // Flag for if background commands are enabled
    // Input Variables
    size_t bufferSize = 0;              // Buffer size of the input
    char* input = NULL;                 // User Input from stdin
    char* arguments[SMALLSH_MAX_ARGS];  // Arguments for User Command;
    // Background Variables
    int numBackPIDs = 0;
    pid_t* backPIDs = malloc(sizeof(pid_t));
    // Signals
    struct sigaction SIGINT_action = {0};   // SIGINT
    struct sigaction SIGTSTP_action = {0};  // SIGTSTP
    
    // Initialize SIGINT_action
    SIGINT_action.sa_handler = SIG_IGN;
    sigfillset(&SIGINT_action.sa_mask);
    SIGINT_action.sa_flags = SA_RESTART;
    sigaction(SIGINT, &SIGINT_action, NULL);

    // Initialize SIGTSTP_action
    SIGTSTP_action.sa_handler = catchSIGTSTP;
    sigfillset(&SIGTSTP_action.sa_mask);
    SIGTSTP_action.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);


    do
    {
        writePrompt();
        getInput(&input, &bufferSize);              // Get Input and Allocate Memory
        replacePID(input);                          // Expand $$ into Process ID of Shell
        tokenizeInput(input, arguments);            // Tokenize Input and Put into Arguments Array
        
        // Check and Perform Command
        if (arguments[0] != NULL && arguments[0][0] != '#')   // Allow Blank Lines and Comments
        {
            // Built-in Command: 'cd'
            if (!strcmp(arguments[0], "cd"))
            {
                smallsh_cd(arguments);
            }
            // Built-in Command: 'status'
            else if (!strcmp(arguments[0], "status"))
            {
                smallsh_status(terminatedNormally, exitStatus);
            }
            // Built-in Command: 'exit'
            else if (!strcmp(arguments[0], "exit"))
            {
                smallsh_exit(input, backPIDs, numBackPIDs);
            }
            // Execute Command
            else
            {
                smallsh_exec(arguments, &backPIDs, &numBackPIDs, &exitStatus, SIGINT_action, &terminatedNormally);
            }
        }

        cleanInput(&input);                         // Deallocate Memory for Input
        resetArguments(arguments);                  // Reset all arguments to NULL
        waitChildren(backPIDs, numBackPIDs);
    } while(1);

    return 0;
}

void writePrompt()
{
    printf(": ");
    fflush(stdout);
}

void getInput(char** inputPtr, size_t* bufferPtr)
{
    int numChars = getline(inputPtr, bufferPtr, stdin);
    (*inputPtr)[numChars - 1] = '\0';
}

void tokenizeInput(char* input, char** arguments)
{
    char* charPtr = input;

    int counter = 0;
    while ((arguments[counter] = strtok(charPtr, " ")) != NULL)
    {
        counter++;
        charPtr = NULL;
    }
}

void replacePID(char* input)
{
    // Set Pointers
    char* ptr = input, *lastPtr = input;

    // Get the Process ID
    char processID[22];
    sprintf(processID, "%ld", (long)getpid());
    int pidLength = strlen(processID);

    // Initialize Buffer
    char buffer[SMALLSH_MAX_CHAR];
    char* bufferPtr = &buffer[0];

    // Repeat until "$$" is no longer found
    while(ptr = strstr(ptr, "$$"))
    {
        // Copy Previous Characters
        memcpy(bufferPtr, lastPtr, (ptr - lastPtr));
        bufferPtr += (ptr - lastPtr);

        // Insert pid
        memcpy(bufferPtr, processID, pidLength);
        bufferPtr += pidLength;

        // Increase Pointers past $$s
        lastPtr = ptr += sizeof(char) * 2;
    }

    strcpy(bufferPtr, lastPtr); // Add remaining text
    strcpy(input, buffer);      // Move buffer to input
}

void resetArguments(char** arguments)
{
    int counter = 0;
    while (arguments[counter] != NULL)
    {
        arguments[counter] = NULL;
        counter++;
    }
}

void cleanInput(char** inputPtr)
{
    free(*inputPtr);
    (*inputPtr) = NULL;
}

void smallsh_cd(char** arguments)
{
    // char directory[100]                      //DEBUGGING
    // printf("%s\n", getcwd(directory, 100));  //DEBUGGING
    // No Arguments
    if (arguments[1] == NULL)
    {
        // Change Directory to HOME environment variable
        chdir(getenv("HOME"));
    }
    // Argument Present
    else
    {
        // Change Directory to Argument.
        chdir(arguments[1]);
    }
    // printf("%s\n", getcwd(directory, 100));  //DEBUGGING
}

void smallsh_status(int terminatedNormally, int exitStatus)
{
    // Check if Child Exited Normally
    if (terminatedNormally)
    {
        printf("exit value %d\n", exitStatus);
    }
    // Otherwise, Return Terminating Signal
    else
    {
        printf("terminated by signal %d\n", exitStatus);
    }
    // Clear stdout
    fflush(stdout);
}

void smallsh_exit(char* input, pid_t* backPIDs, int numPIDs)
{
    // Cleanup
    cleanInput(&input);

    // Go through all the background processes
    int index;
    for (index = 0; index < numPIDs; index++)
    {
        // Check if process is still running
        int childExitMethod;
        pid_t actualPid = waitpid(backPIDs[index], &childExitMethod, WNOHANG);
        // If its running, kill the signal
        if(!actualPid)
        {
            kill(backPIDs[index], SIGKILL);
        }
    } 

    // Deallocate background PIDs
    free(backPIDs);
    
    // Terminate
    exit(0);
}

int _findString(char** array, char* string)
{
    int count = 0;
    while (array[count] != NULL)
    {
        // printf("%d: '%s'\n", count, array[count]);
        if(!strcmp(array[count], string))
        {
            return count;
        }
        count++;
    }
    return -1;
}

int checkRedirectOut(char** arguments)
{
    return _findString(arguments, ">");
}

int checkRedirectIn(char** arguments)
{
    return _findString(arguments, "<");
}

void redirectInSetup(int index, int* source, char** arguments)
{
    if (index > 0)
    {
        char* inputFileName = arguments[index + 1];

        // open source file
        *source = open(inputFileName, O_RDONLY);
        if (*source == -1)
        {
            printf("cannot open source for input\n");
            fflush(stdout);
            exit(1);
        }
        fcntl(*source, F_SETFD, FD_CLOEXEC);     // Close file if it is executed.
        // redirect stdin to source
        int result = dup2(*source, 0);
        if (result == -1)
        {
            perror("dup2(source, 0) has failed\n");
            exit(1);
        }

        // Setup Argument for Execution if Redirection Successful
        if (*source != -1 && result != -1)
        {
            arguments[index] = '\0';
        }
    }
}

void redirectOutSetup(int index, int* target, char** arguments)
{
    if (index > 0)
    {
        char* outputFileName = arguments[index + 1];

        // open target file
        *target = open(outputFileName, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (*target == -1)
        {
            printf("cannot open target for output\n");
            fflush(stdout);
            exit(2);
        }
        fcntl(*target, F_SETFD, FD_CLOEXEC);     // Close file if it is executed.
        // redirect stdin to source
        int result = dup2(*target, 1);
        if (result == -1)
        {
            perror("dup2(target, 1) has failed\n");
            exit(2);
        }
        // Setup Argument for Execution if Redirection Successful
        if (*target != -1 && result != -1)
        {
            arguments[index] = '\0';
        }
    }
}

void redirectSetupBG(int rInIndex, int rOutIndex, int* source, int* target)
{
    if (rInIndex < 0)
    {
        // Open /dev/null
        *source = open("/dev/null", O_RDONLY);
        if (*source == -1)
        {
            printf("cannot open source for input\n");
            fflush(stdout);
            exit(1);
        }
        fcntl(*source, F_SETFD, FD_CLOEXEC);     // Close file if it is executed.
        // redirect stdin to source
        int result = dup2(*source, 0);
        if (result == -1)
        {
            perror("dup2(source, 0) has failed\n");
            exit(1);
        }
    }
    if (rOutIndex < 0)
    {
        // open target file
        *target = open("/dev/null", O_WRONLY);
        if (*target == -1)
        {
            printf("cannot open target for output\n");
            fflush(stdout);
            exit(2);
        }
        fcntl(*target, F_SETFD, FD_CLOEXEC);     // Close file if it is executed.
        // redirect stdin to source
        int result = dup2(*target, 1);
        if (result == -1)
        {
            perror("dup2(target, 1) has failed\n");
            exit(2);
        }
    }
}

int executeCmd(char** arguments)
{
    if (execvp(*arguments, arguments) < 0)
    {
        // TODO: Fix Output
        perror("");
        return 2;
    }
    return 0;
}

int smallsh_exec(char** arguments, pid_t** backPIDs, int* numPIDs, int* exitStatus, struct sigaction sigact, int* termNormal)
{
    pid_t spawnPid = -3;
    int childExitMethod = -3;

    // Check if Background Command
    int backCmd = isBackground(arguments);

    // Fork a Child
    spawnPid = fork();

    switch (spawnPid)
    {
        // Send Error if Fork Failed.
        case -1:
        {   perror("fork() failed\n");
            exit(1);
            break;
        }
        // Execute Command If Child
        case 0:
        {
            // Set Signal Handler so Singal Terminates if its a Foreground Command
            if (!backCmd)
            {
                sigact.sa_handler = SIG_DFL;
                sigaction(SIGINT, &sigact, NULL);
            }

            // Setup Redirection if Redirection Detected
            int source = -3,    // The Source File Descriptor
                target = -3,    // The Target File Descriptor
                redirectInIndex = checkRedirectIn(arguments),   // Index of "<" character
                redirectOutIndex = checkRedirectOut(arguments); // Index of ">" character
            if (redirectInIndex > 0 || redirectOutIndex > 0)
            {
                // If Valid, Setup stdin
                redirectInSetup(redirectInIndex, &source, arguments);
                // If Valid, Setup stdout
                redirectOutSetup(redirectOutIndex, &target, arguments);
            }
            // If its a background command, redirect to /dev/null
            if (backCmd)
            {
                redirectSetupBG(redirectInIndex, redirectOutIndex, &source, &target);
            }
            // Execute the Command
            executeCmd(arguments);
            exit(3);
            break;
        }
        // Let Parent Process Continue Running
        default:
        {
            // If Foreground Command, wait for exit and store exit status
            if (!backCmd)
            {
                // If Foreground Command, wait for completion
                pid_t actualPid = waitpid(spawnPid, &childExitMethod, 0);
                // If the Process Didn't Exit Normally, inform user
                if (!WIFEXITED(childExitMethod))
                {
                    // Inform user
                    printf("terminated by signal %d\n", WTERMSIG(childExitMethod));
                    fflush(stdout);
                    // Set Normal Termination Flag to False
                    *termNormal = 0;
                    // Set Exit Status
                    *exitStatus = WTERMSIG(childExitMethod);
                }
                else
                {
                    // Set Normal Termination Flag to True
                    *termNormal = 1;
                    // Set Exit Status
                    *exitStatus = WEXITSTATUS(childExitMethod);
                }
            }
            // If Background Command, print and save the PID, then continue
            else
            {
                printf("background pid is %d\n", spawnPid);
                fflush(stdout);
                savePID(backPIDs, numPIDs, spawnPid);
            }

            return 0;
            break;
        }
    }
}

void savePID(pid_t** array, int* size, pid_t input)
{
    // If the array has values, create space for a new one.
    if (*size > 0)
    {
        // Allocate Memory for Temp Array
        pid_t* tempArray = malloc((*size + 1) * sizeof(pid_t));
        // Add Previous Values to Temp Array
        int index;
        for (index = 0; index < *size; index++)
        {
            tempArray[index] = (*array)[index];
        }
        // Add Input to Temp Array
        tempArray[*size] = input;
        // Deallocate Memory for Array
        free(*array);
        // Set Array to tempArray
        *array = malloc((*size + 1) * sizeof(pid_t));
        for (index = 0; index <= (*size); index++)
        {
            (*array)[index] = tempArray[index];
        }
        // Deallocate Temp Array
        free(tempArray);
    }
    // If the array is empty
    else
    {
        (*array)[0] = input;
    }
    // Increment the size of the array
    (*size)++;
}

int isBackground(char** arguments)
{
    // Get the Last Index
    int index;
    for(index = 0; arguments[index]; index++)
    {
        continue;
    }
    index--;
    // Check if Last Argument is &
    if (!strcmp(arguments[index], "&"))
    {
        // Clear & for Command processing
        arguments[index] = NULL;
        
        // If background commands enabled
        if(backgroundEnabled)
        {
            // Show that background commands are present
            return 1;
        }
    }
    return 0;
}

void waitChildren(pid_t* array, int size)
{
    // Go through each child process and wait for finished children
    int index;
    int childExitMethod;
    for (index = 0; index < size; index++)
    {
        pid_t actualPid = waitpid(array[index], &childExitMethod, WNOHANG);
        if(actualPid && actualPid > 0)
        {
            printf("background pid %d is done: exit value %d\n", actualPid, WEXITSTATUS(childExitMethod));
            fflush(stdout);
        }
    }
}

void catchSIGTSTP(int signo)
{
    if(backgroundEnabled)
    {
        backgroundEnabled = 0;
        char* message = "\nEntering foreground-only mode (& is now ignored)\n: ";
        write(STDOUT_FILENO, message, 52);
    }
    else
    {
        backgroundEnabled = 1;
        char* message = "\nExiting foreground-only mode\n: ";
        write(STDOUT_FILENO, message, 32);
    }
    fflush(stdout);
}