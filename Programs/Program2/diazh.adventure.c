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
    char _connectNames[6][8];       // The Names of Connected Rooms
};

int getNewestDirectory(char* directoryPrefix, char* directoryName);
void initializeRooms(struct Room* rooms, int numRooms);
int fileToRoom(FILE* fileInput, struct Room* rooms, int* roomCounter);
void setRoomConnections(struct Room* rooms, int numRooms);
int populateRooms(char* directoryName, char* fileType, struct Room* rooms);
void _printRooms(struct Room* rooms, int numRoms);
int _getStartIndex(struct Room* rooms, int numRooms);
void _printInterface(struct Room* room);
int _getValidateInput(struct Room** room, char* input, int bufferSize);
int _checkEnd(struct Room* room);
void _displayEndMessage(int steps, char** roomsVisited);

int playGame(struct Room* rooms, int numRooms)
{
    int steps = 0;          // Counter for the Number of Steps
    char** traveledRooms;   // Holds Previously Visited Rooms
    traveledRooms = malloc(sizeof(char*) * (steps + 1));

    // Find the Starting Room
    int startRoomIndex = _getStartIndex(rooms, numRooms);
    struct Room* curRoom = &rooms[startRoomIndex];

    // Play the Game Until the End Room is Found
    int play = 1;
    while(play)
    {
        // Display Interface
        _printInterface(curRoom);

        // Get User Input
        const int BUFFER_SIZE = 256;
        char input[BUFFER_SIZE];
        int inputValid = _getValidateInput(&curRoom, input, BUFFER_SIZE);

        // Inform User if Input is Invalid
        if(inputValid == 0)
        {
            printf("\nHUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
        }
        // Otherwise, Increase Number of Steps and Check if in End Room
        else
        {
            steps++;                    // Increment the Number of Steps
            play = !_checkEnd(curRoom); // Check if Current Room is the End

            // Record the Path of the User
            // If its the first step, set the first index
            if (steps == 1)
            {
                traveledRooms[steps - 1] = curRoom->name;
            }
            // Otherwise, reallocate the array and add the new room
            else
            {
                // Create a temporary array to hold the previous rooms
                char** tempRooms = malloc(sizeof(char*) * (steps - 1));
                int count;
                for(count = 0; count < steps - 1; count++)
                {
                    tempRooms[count] = traveledRooms[count];
                }

                // Reallocate Memory and Populate with Previous Rooms
                free(traveledRooms);        // Clear Old Traveled Room
                traveledRooms = malloc(sizeof(char*) * steps);
                for(count = 0; count < steps - 1; count++)
                {
                    traveledRooms[count] = tempRooms[count];
                }

                // Add the Newest Room
                traveledRooms[steps - 1] = curRoom->name;

                free(tempRooms);            // Clear the Temporary Array
            }
        }

        printf("\n");       // Create whitespace for new iteration
    }

    // Display End Game Sceen
    _displayEndMessage(steps, traveledRooms);

    free(traveledRooms);    // Clear the list of visited rooms

    return 0;
}

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

    // Transfer Room Data from Files into Structs
    exitStatus = populateRooms(directoryName, ROOM_FILE_TYPE, rooms);

    // _printRooms(rooms, NUM_ROOMS);  // DEBUGGING: View Room Info

    playGame(rooms, NUM_ROOMS);

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
            strcpy((rooms[*roomCounter]._connectNames)[rooms[*roomCounter].numConnections], token3);
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

void setRoomConnections(struct Room* rooms, int numRooms)
{
    // Go throuch each room
    int roomCount;
    for(roomCount = 0; roomCount < numRooms; roomCount++)
    {
        int connectCount;
        // Go through each connection for the room
        for(connectCount = 0; connectCount < rooms[roomCount].numConnections; connectCount++)
        {
            char* roomName = rooms[roomCount]._connectNames[connectCount];

            // Search for Connected Room
            int connectRoomNum;
            for(connectRoomNum = 0; connectRoomNum < numRooms; connectRoomNum++)
            {
                // When the connected room name is found, store it
                if(!strcmp(roomName, rooms[connectRoomNum].name))
                {
                    rooms[roomCount].connections[connectCount] = &rooms[connectRoomNum];
                    break;
                }
            }
        }
    }
}

int populateRooms(char* directoryName, char* fileType, struct Room* rooms)
{
    int exitStatus = 0;                                 // Current Exit Status
    int numRooms = 0;                                   // Current Number of Rooms
    int directoryLength = strlen(directoryName) + 2;    // Length of Directory Location
    char directoryLocation[directoryLength];            // Directory Location
    struct dirent* file;                                // The File in the Subdirectory
    struct stat attributes;                             // The Attributes of the File

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

    // Use the _connectName data members from struct to set connections data member.
    setRoomConnections(rooms, numRooms);

    return exitStatus;
}

void _printRooms(struct Room* rooms, int numRooms)
{
    int count;
    for(count = 0; count < numRooms; count++)
    {
        printf("==ROOM %d==\n", count + 1);
        printf("ROOM NAME: %s\n", rooms[count].name);
        // printf("NUM CONNECTIONS: %d\n", rooms[count].numConnections);
        int conCount;
        // for(conCount = 0; conCount < rooms[count].numConnections; conCount++)
        // {
        //     printf("CONNECTION %d NAME: %s\n", conCount, rooms[count]._connectNames[conCount]);
        // }

        for(conCount = 0; conCount < rooms[count].numConnections; conCount++)
        {
            char* roomName = rooms[count].connections[conCount]->name;
            printf("CONNECTION %d: %s\n", conCount + 1, roomName);
        }

        printf("ROOM TYPE: %s\n", rooms[count].type);
    }
}

int _getStartIndex(struct Room* rooms, int numRooms)
{
    int roomNum = -1;

    // Go through each room, looking for start room
    for(roomNum = 0; roomNum < numRooms; roomNum++)
    {
        // When start room found, return the index
        if(!strcmp(rooms[roomNum].type, "START_ROOM"))
        {
            return roomNum;
        }
    }
    // Return error if start room not found.
    if(roomNum == -1)
    {
        fprintf(stderr, "ERROR: Failed to Find Start Room");
        exit(6);
    }
}

void _printInterface(struct Room* room)
{
    // Print Room Name
    printf("CURRENT LOCATION: %s\n",room->name);
    // Print Connected Rooms
    printf("POSSIBLE CONNECTIONS: ");
    int count;
    for(count = 0; count < room->numConnections; count++)
    {
        if(count != room->numConnections - 1)
        {
            printf("%s, ", room->connections[count]->name);
        }
        else
        {
            printf("%s.\n", room->connections[count]->name);
        }
        
    }
    // Ask For Input
    printf("WHERE TO? >");
}

int _getValidateInput(struct Room** room, char* input, int bufferSize)
{
    int inputValid = 0;

    fgets(input, bufferSize, stdin);    // Get Input

    // Replace trailing newline character with null character
    /* Based on 
    https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input */
    char* newline = strchr(input, '\n');
    if (newline != NULL)
    {
        *newline = '\0';
    }

    // Check Each Connected Room for Matching Name
    int count;
    for(count = 0; count < (*room)->numConnections; count++)
    {
        if(!strcmp(input, (*room)->connections[count]->name))
        {
            *room = (*room)->connections[count]; // Set Room
            inputValid = 1;
            break;
        }
    }

    return inputValid;
}

int _checkEnd(struct Room* room)
{
    if(!strcmp(room->type, "END_ROOM"))
    {
        return 1;
    }
    return 0;
}

void _displayEndMessage(int steps, char** roomsVisited)
{
    printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
    printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", steps);
    int count;
    for(count = 0; count < steps; count++)
    {
        printf("%s\n", roomsVisited[count]);
    }
}