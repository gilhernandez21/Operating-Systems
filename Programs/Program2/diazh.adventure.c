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
    char connectNames[6][8];
};

int getNewestDirectory(char* directoryPrefix, char* directoryName);
void initializeRooms(struct Room* rooms, int numRooms);
int fileToRoom(FILE* fileInput, struct Room* rooms, int* roomCounter);
int setRoomConnections(struct Room* rooms);
int populateRooms(char* directoryName, char* fileType, struct Room* rooms);
void _printRooms(struct Room* rooms, int numRoms);

int main()
{
    const int NUM_ROOMS = 7;                    // The Number of Rooms for the Program
    char* DIRECTORY_PREFIX = "diazh.rooms.";    // Prefix of Directory
    char* ROOM_FILE_TYPE = ".room";             // File type of Room Files
    const int DIR_NAME_LENGTH = 100;            // Length Of Directory Name

    int exitStatus = 0;                         // Exit Status of the Program
    char directoryName[DIR_NAME_LENGTH];        // Directory of Rooms

    struct Room* rooms;                                 // The Rooms for the Game
    rooms = malloc(NUM_ROOMS * sizeof(struct Room));    // Allocate Memory
    initializeRooms(rooms, NUM_ROOMS);                  // Initialize the Rooms

    // Get Newest Directory
    memset(directoryName, '\0', DIR_NAME_LENGTH * sizeof(char));
    exitStatus = getNewestDirectory(DIRECTORY_PREFIX, directoryName);

    // Store Rooms
    exitStatus = populateRooms(directoryName, ROOM_FILE_TYPE, rooms);

    _printRooms(rooms, NUM_ROOMS);  // DEBUGGING

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

void initializeRooms(struct Room* rooms, int numRooms)
{
    int count;
    for(count = 0; count < numRooms; count++)
    {
        rooms[count].id = count;            // Set Room ID
        rooms[count].numConnections = 0;    // Set Number of Connections
    }
}

int fileToRoom(FILE* fileInput, struct Room* rooms, int* roomCounter)
{
    const int BUFFER_SIZE = 256;    // Number of Characters for Buffer
    const int TOKEN_SIZE = 32;      // Number of Characters for Tokens
    int exitStatus = 0;             // Remains 0 Unless Error Occurs

    int nameFound = 0;      // Whether a name variable was found
    int typeFound = 0;      // Whether the type was found
    int numConnections = 0; // Counter for the Number of Connections

    char buffer[BUFFER_SIZE];       // The buffer

    char* NAME_S = "NAME:";         // Determines Room Name
    char* TYPE_S = "TYPE:";         // Determines Room Type
    char* CONN_S = "CONNECTION";    // Determines a Room Connection

    // Go through each line of the file
    while(fgets(buffer, sizeof(buffer), fileInput))
    {
        char token1[TOKEN_SIZE], token2[TOKEN_SIZE], token3[TOKEN_SIZE];
        sscanf(buffer, "%s %s %s", token1, token2, token3);

        // If the buffer holds the name, store it in room's name.
        if(!strcmp(token2, NAME_S))
        {
            nameFound = 1;
            strcpy(rooms[*roomCounter].name, token3);
        }
        // If the buffer holds the type, store it in the room's type.
        else if (!strcmp(token2, TYPE_S))
        {
            typeFound = 1;
            strcpy(rooms[*roomCounter].type, token3);
        }
        // If the buffer holds a connection, save the name of the connection
        else if (!strcmp(token1, CONN_S))
        {
            strcpy((rooms[*roomCounter].connectNames)[rooms[*roomCounter].numConnections], token3);
            rooms[*roomCounter].numConnections++;
        }
        else
        {
            fprintf(stderr, "ERROR: Invalid Line Detected.");
            fprintf(stderr, "%s %s %s", token1, token2, token3);
            exitStatus = 4;
            break;
        }
    }

    // If file failed to produce any names, types, or improper connections - show error
    if(nameFound == 0 || typeFound == 0 || rooms[*roomCounter].numConnections < 3 || rooms[*roomCounter].numConnections > 6)
    {
        fprintf(stderr, "ERROR: Room '%s' Invalid!", rooms[*roomCounter].name);
        exitStatus = 5;
    }

    if(exitStatus != 0)
    {
        exit(exitStatus);
    }

    ++*roomCounter; // increment the room counter

    return exitStatus;
}

int populateRooms(char* directoryName, char* fileType, struct Room* rooms)
{
    int exitStatus = 0;                                 // Current Exit Status
    int numRooms = 0;                                   // Current Number of Rooms
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

            // Convert File to Room if File Was Opened
            if (fileInput != NULL)
            {
                // printf("==ROOM %d==\n", numRooms + 1);

                exitStatus = fileToRoom(fileInput, rooms, &numRooms);
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

    if(exitStatus != 0)
    {
        exit(exitStatus);
    }
    return exitStatus;
}

void _printRooms(struct Room* rooms, int numRooms)
{
    int count;
    for(count = 0; count < numRooms; count++)
    {
        printf("==ROOM %d==\n", count + 1);
        printf("ROOM NAME: %s\n", rooms[count].name);
        printf("ROOM TYPE: %s\n", rooms[count].type);

        printf("NUM CONNECTIONS: %d\n", rooms[count].numConnections);

        int conCount;
        for(conCount = 0; conCount < rooms[count].numConnections; conCount++)
        {
            printf("CONNECTION %d NAME: %s\n", conCount, rooms[count].connectNames[conCount]);
        }
    }
}
