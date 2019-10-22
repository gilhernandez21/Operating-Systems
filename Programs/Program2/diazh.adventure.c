#include <dirent.h>
#include <fcntl.h>
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
int populateRooms(char* directoryName, char* fileType, struct Room* rooms);

int main()
{
    const int NUM_ROOMS = 7;                    // The Number of Rooms for the Program
    char* DIRECTORY_PREFIX = "diazh.rooms.";    // Prefix of Directory
    char* ROOM_FILE_TYPE = ".room";             // File type of Room Files
    const int DIR_NAME_LENGTH = 100;            // Length Of Directory Name

    int exitStatus = 0;                         // Exit Status of the Program
    char directoryName[DIR_NAME_LENGTH];        // Directory of Rooms
    struct Room* rooms;                         // The Rooms for the Game

    // Get Newest Directory
    memset(directoryName, '\0', DIR_NAME_LENGTH * sizeof(char));
    exitStatus = getNewestDirectory(DIRECTORY_PREFIX, directoryName);

    // Store Rooms
    rooms = malloc(NUM_ROOMS * sizeof(struct Room));    // Allocate Memory
    exitStatus = populateRooms(directoryName, ROOM_FILE_TYPE, rooms);


    free(rooms);    // Deallocate Memory

    return exitStatus;
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

int populateRooms(char* directoryName, char* fileType, struct Room* rooms)
{
    int exitStatus = 0;                                 // Current Exit Status
    int directoryLength = strlen(directoryName) + 2;    // Length of Directory Location
    char directoryLocation[directoryLength];            // Directory Location
    struct dirent* file;
    struct stat attributes;

    // Create Directory Location
    memset(directoryLocation, '\0', directoryLength * sizeof(char));
    sprintf(directoryLocation, "./%s", directoryName);

    DIR* directory = opendir(directoryLocation);    // Open Directory

    // Go through each file in the directory
    while((file = readdir(directory)) != NULL)
    {
        char* extension = strrchr(file->d_name, '.');   // Get the File Extension

        // If the correct file is found
        if (!strcmp(extension, fileType) && extension != NULL)
        {
            // Get File Path
            int PATH_LENGTH = strlen(directoryLocation) + strlen(file->d_name) + 1;
            char filePath[PATH_LENGTH];
            memset(filePath, '\0', PATH_LENGTH * sizeof(char));
            sprintf(filePath, "%s/%s", directoryLocation, file->d_name);

            FILE* fileInput = fopen(filePath, "r"); // Open File

            if (fileInput != NULL)
            {
                // TODO: Get Information, Store Into rooms
                printf("%s\n", file->d_name);

                int LINE_LENGTH = 256;
                char line[LINE_LENGTH];
                int count = 0;

                while(fgets(line, sizeof(line), fileInput))
                {
                    printf("Line %d: %s", ++count, line);
                }
            }
            else
            {
                fprintf(stderr, "ERROR: Failed to Open File '%s'.", file->d_name);
                exitStatus = 3;
                break;
            }
            
            fclose(fileInput);                      // Close File
        }
    }

    closedir(directory);        // Close Directory

    return exitStatus;
}
