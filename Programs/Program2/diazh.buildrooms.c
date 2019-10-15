#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

struct room
{
    char* name;
    char* type;
    int numConnections;
    struct room* connections[6];
};

int makeDirectory(char* directoryName)
{
    // Make Directory
    int result = mkdir(directoryName, 0755);

    // Check for Failure
    if (result != 0)
    {
        fprintf(stderr, "ERROR: Failed to Make Directory. \'mkdir()\' returned: %d", result);
        exit(1);
    }
    
    return 0;
}

int _generateRoomFile(char* directory, char* file, char* input)
{
    // generate location of file
    char location[64];
    sprintf(location, "./%s/%s", directory, file);

    // open file
    int fileDescriptor = open(location, O_WRONLY | O_TRUNC | O_CREAT, 0600);

    if (fileDescriptor < 0)
    {
        fprintf(stderr, "ERROR: Failed to Make File '%s'.", file);
        exit(2);
    }

    ssize_t nwritten = write(fileDescriptor, input, strlen(input) * sizeof(char));

    // close file
    close(fileDescriptor);

    return 0;
}

int main()
{
    // Generate Directory Name
    char directoryName[32];
    pid_t pid = getpid();
    sprintf(directoryName, "diazh.rooms.%d", pid);

    // Make Directory
    makeDirectory(directoryName);

    return 0;
}