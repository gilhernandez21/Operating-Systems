#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

#define SMALLSH_MAX_ARGS 512

void writePrompt();
void getInput(char** inputPtr, size_t* bufferPtr);
void tokenizeInput(char* input, char** arguments);
void resetArguments(char** arguments);
void cleanInput(char** inputPtr);
void smallsh_cd(char** arguments);
void smallsh_status(int exitStatus);
void smallsh_exit(char* input);

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
    char* arguments[SMALLSH_MAX_ARGS];  // Arguments for User Command

    do
    {
        writePrompt();
        getInput(&input, &bufferSize);              // Get Input and Allocate Memory
        tokenizeInput(input, arguments);            // Tokenize Input and Put into Arguments Array
        checkInput(input, arguments, &exitStatus);

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