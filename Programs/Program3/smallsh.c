#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#define SMALLSH_MAX_ARGS 512

void writePrompt();
void getInput(char** inputPtr, size_t* bufferPtr);
void inputToComArgs(char* input, char** commandPtr, char** arguments);
void resetArguments(char** arguments);
void cleanInput(char** inputPtr);
void smallsh_cd(char** arguments);
void smallsh_status(int exitStatus);
void smallsh_exit(char* input);

int checkInput(char* input, char* command, char** arguments, int* exitStatus)
{
    if (command != NULL && command[0] != '#')   // Allow Blank Lines and Comments
    {
        // Built-in Command: 'cd'
        if (!strcmp(command, "cd"))
        {
            smallsh_cd(arguments);
        }
        // Built-in Command: 'status'
        else if (!strcmp(command, "status"))
        {
            smallsh_status(*exitStatus);
        }
        // Built-in Command: 'exit'
        else if (!strcmp(command, "exit"))
        {
            smallsh_exit(input);
        }
        else
        {
            printf("==TODO: PERFORM 'exec' FUNCTION==\n");
        }
        
    }

    return 1;
}

int main()
{
    int exitStatus = 0;
    size_t bufferSize = 0;
    char* input = NULL;                 // User Input from stdin
    char* command;                      // User Command
    char* arguments[SMALLSH_MAX_ARGS];  // Arguments for User Command

    do
    {
        writePrompt();
        getInput(&input, &bufferSize);              // Get Input and Allocate Memory
        inputToComArgs(input, &command, arguments); // Separate Command and Arguments from Input
        checkInput(input, command, arguments, &exitStatus);

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

void inputToComArgs(char* input, char** commandPtr, char** arguments)
{
    *commandPtr = strtok(input, " ");

    int counter = 0;
    while ((arguments[counter] = strtok(NULL, " ")) != NULL)
    {
        counter++;
    }
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

void smallsh_cd(char** arguments)
{
    // char directory[100]                      //DEBUGGING
    // printf("%s\n", getcwd(directory, 100));  //DEBUGGING
    // No Arguments
    if (arguments[0] == NULL)
    {
        // Change Directory to HOME environment variable
        chdir(getenv("HOME"));
    }
    // Argument Present
    else
    {
        // Change Directory to Argument.
        chdir(arguments[0]);
    }
    // printf("%s\n", getcwd(directory, 100));  //DEBUGGING
}

void cleanInput(char** inputPtr)
{
    free(*inputPtr);
    (*inputPtr) = NULL;
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