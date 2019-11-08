#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SMALLSH_MAX_ARGS 512

/* PROTOTYPES **/
// Display Prompt
void writePrompt();
// Get and Translate User Input
void getInput(char** inputPtr, size_t* bufferPtr);
void tokenizeInput(char* input, char** arguments);
// Check and Process Commands
int checkInput(char* input, char** arguments, int* exitStatus);
// Redirection Functions
int _findString(char** array, char* string);
int checkRedirectOut(char** arguments);
int checkRedirectIn(char** arguments);
void redirectInSetup(int index, int* source, char** arguments);
void redirectOutSetup(int index, int* target, char** arguments);
// Execution Functions
int executeCmd(char** arguments);
int smallsh_exec(char** arguments);
// Built-In Functions
void smallsh_cd(char** arguments);
void smallsh_status(int exitStatus);
void smallsh_exit(char* input);
// Cleanup Functions
void resetArguments(char** arguments);
void cleanInput(char** inputPtr);

int main()
{
    int exitStatus = 0;
    size_t bufferSize = 0;
    char* input = NULL;                 // User Input from stdin
    char* arguments[SMALLSH_MAX_ARGS];  // Arguments for User Command

    do
    {
        writePrompt();
        getInput(&input, &bufferSize);              // Get Input and Allocate Memory
        tokenizeInput(input, arguments);            // Tokenize Input and Put into Arguments Array
        checkInput(input, arguments, &exitStatus);  // Check and Perform Command

        cleanInput(&input);                         // Deallocate Memory for Input
        resetArguments(arguments);                  // Reset all arguments to NULL
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

int checkInput(char* input, char** arguments, int* exitStatus)
{
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
            smallsh_status(*exitStatus);
        }
        // Built-in Command: 'exit'
        else if (!strcmp(arguments[0], "exit"))
        {
            smallsh_exit(input);
        }
        // Execute Command
        else
        {
            smallsh_exec(arguments);
        }
        
    }

    return 1;
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

void smallsh_status(int exitStatus)
{
    printf("exit value %d\n", exitStatus);
    fflush(stdout);
}

void smallsh_exit(char* input)
{
    // Cleanup
    cleanInput(&input);
    
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
            perror("cannot open source for input\n");
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
            perror("cannot open target for output\n");
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

int executeCmd(char** arguments)
{
    if (execvp(*arguments, arguments) < 0)
    {
        perror("");
        return 2;
    }
    return 0;
}

int smallsh_exec(char** arguments)
{
    pid_t spawnPid = -3;
    int childExitMethod = -3;

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
            int source = -3,    // The Source File Descriptor
                target = -3,    // The Target File Descriptor
                redirectInIndex = checkRedirectIn(arguments),   // Index of "<" character
                redirectOutIndex = checkRedirectOut(arguments); // Index of ">" character

            // Setup Redirection if Redirection Detected
            if (redirectInIndex > 0 || redirectOutIndex > 0)
            {
                // If Valid, Setup stdin
                redirectInSetup(redirectInIndex, &source, arguments);
                // If Valid, Setup stdout
                redirectOutSetup(redirectOutIndex, &target, arguments);
            }
            // Execute the Command
            executeCmd(arguments);
            exit(3);
            break;
        }
        // Let Parent Process Continue Running
        default:
        {
            pid_t actualPid = waitpid(spawnPid, &childExitMethod, 0);
            return 0;
            break;
        }
    }
}