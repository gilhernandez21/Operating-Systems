#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

struct Room
{
    int id;                         // ID OF THE ROOM
    char name[8];                   // NAME OF THE ROOM
    char type[12];                  // TYPE OF ROOM
    int numConnections;             // NUMBER OF CONNECTIONS TO ROOM
    struct Room* connections[6];    // THE CONNECTED ROOMS
};

int getNewestDirectory(char* directoryPrefix, char* directoryName);

int main()
{
    char* DIRECTORY_PREFIX = "diazh.rooms.";  // Prefix of Directory
    const int DIR_NAME_LENGTH = 100;

    // Get Newest Directory
    char directoryName[DIR_NAME_LENGTH];
    memset(directoryName, '\0', DIR_NAME_LENGTH);
    getNewestDirectory(DIRECTORY_PREFIX, directoryName);

    return 0;
}

int getNewestDirectory(char* directoryPrefix, char* directoryName)
{
    int exitStatus = 0;                     // The Exit Status of Function
    DIR* startDirectory = opendir(".");     // Starting directory
    struct dirent* subdirectory;            // Subdirectory
    struct stat attributes;                 // Stats for the newest subdirectory
    int newestSubTime = -1;                 // Newest subdirectory time

    // Check if directory could be opened.
    if (startDirectory > 0)
    {
        int validDirFound = 0;  // By default, there is no valid directory.

        // Go through all subdirectories
        while((subdirectory = readdir(startDirectory)) != NULL)
        {
            // Get prefix if it is present in subdirectory name
            char* prefixPresent = strstr(subdirectory->d_name, directoryPrefix);
            // If prefix is present
            if (prefixPresent)
            {
                validDirFound = 1;  // Confirm Valid Directory was found

                char* currentSubName = subdirectory->d_name;    // the current subdirectory name
                stat(currentSubName, &attributes);              // Store subdirectory attributes
                int currentSubTime = (int)attributes.st_mtime;  // the current subdirectory time

                // Check if current subdirectory is newer than previous newest subdirectory
                if (newestSubTime < currentSubTime)
                {
                    newestSubTime = currentSubTime;                     // Set Newest Directory Time
                    memset(directoryName, '\0', sizeof(directoryName)); // Reset Newest Directory Name
                    strcpy(directoryName, currentSubName);              // Save Newest Directory Name
                }
            }
        }

        // Print Error if no valid directory was found.
        if(!validDirFound)
        {
            fprintf(stderr, "ERROR: Failed to Locate Valid Directories.\n");
            fprintf(stderr, "Run Buildrooms Program to Create '%s' Directories.", directoryPrefix);
            exitStatus = 2;
        }
    }
    // Print Error if Start Directory Failed to Open.
    else
    {
        fprintf(stderr, "ERROR: Failed to Open Current Directory.");
        exitStatus = 1;
    }

    closedir(startDirectory);   // Close the Opened Directory

    if(exitStatus != 0)
    {
        exit(exitStatus);
    }
    return exitStatus;
}

