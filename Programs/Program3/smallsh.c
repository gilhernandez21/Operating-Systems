#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void writePrompt();
void getInput(char** inputPtr, size_t* bufferPtr);
void cleanInput(char** inputPtr);
void smallsh_cd(char* token);
void smallsh_status(int exitStatus);
void smallsh_exit(int* runPtr);

int checkInput(char* input, int* exitStatus, int* run)
{   
    char* token = strtok(input, " ");
    if (token != NULL && token[0] != '#')   // Allow Blank Lines and Comments
    {
        // Built-in Command: 'cd'
        if (!strcmp(token, "cd"))
        {
            smallsh_cd(token);
        }
        // Built-in Command: 'status'
        else if (!strcmp(token, "status"))
        {
            smallsh_status(*exitStatus);
        }
        // Built-in Command: 'exit'
        else if (!strcmp(token, "exit"))
        {
            smallsh_exit(run);
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
    int run = 1;
    size_t bufferSize = 0;
    char* input = NULL;

    do
    {
        writePrompt();
        getInput(&input, &bufferSize);          // Get Input and Allocate Memory
        checkInput(input, &exitStatus, &run);   // Check Input

        cleanInput(&input);                     // Deallocate Memory for Input
    } while(run);

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

void smallsh_cd(char* token)
{
    token = strtok(NULL, " ");  // Check for Argument

    // No Arguments
    if (token == NULL)
    {
        // Change Directory to HOME environment variable
        chdir(getenv("HOME"));
    }
    // Argument Present
    else
    {
        // Change Directory to token.
        chdir(token);
    }
    
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

void smallsh_exit(int* runPtr)
{
    // Cleanup

    // Terminate
    (*runPtr) = 0;
}