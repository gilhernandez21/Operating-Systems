#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void getNewestDirectory(char* directoryPrefix, char* directoryName);

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

void getNewestDirectory(char* directoryPrefix, char* directoryName)
{
    DIR* startDirectory = opendir(".");     // Holds the starting directory
    struct dirent* subdirectory;            // Holds the subdirectory
    struct stat attributes;                 // Holds the stats for the newest subdirectory
    int newestSubTime = -1;                 // Holds the newest subdirectory time

    // Check if directory could be opened.
    if (startDirectory > 0)
    {
        int validDirFound = 0;

        while((subdirectory = readdir(startDirectory)) != NULL)
        {
            // Get prefix if it is present in subdirectory name
            char* prefixPresent = strstr(subdirectory->d_name, directoryPrefix);
            // If prefix is present
            if (prefixPresent != NULL)
            {
                validDirFound = 1;

                char* currentSubName = subdirectory->d_name;    // the current subdirectory name

                // Store subdirectory attributes
                stat(currentSubName, &attributes);

                int currentSubTime = (int)attributes.st_mtime;  // the current subdirectory time

                // Check if current subdirectory is newer than previous newest subdirectory
                if (newestSubTime < currentSubTime)
                {
                    // Set Newest Directory Time
                    newestSubTime = currentSubTime;
                    // Reset Newest Directory Name
                    memset(directoryName, '\0', sizeof(directoryName));
                    // Save Newest Directory Name
                    strcpy(directoryName, currentSubName);
                }
            }
        }

        if(!validDirFound)
        {
            fprintf(stderr, "ERROR: Failed to Locate Valid Directories.\n");
            fprintf(stderr, "Run Buildrooms Program to Create '%s' Directories.", directoryPrefix);
            exit(2);
        }
    }
    // Print Error if Start Directory Failed to Open.
    else
    {
        fprintf(stderr, "ERROR: Failed to Open Current Directory.");
        exit(1);
    }
}