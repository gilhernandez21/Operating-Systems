#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

struct Room
{
    char name[8];                   // NAME OF THE ROOM
    char type[12];                  // TYPE OF ROOM
    int numConnections;             // NUMBER OF CONNECTIONS TO ROOM
    struct room* connections[6];    // THE CONNECTED ROOMS
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

int generateEndIndex(int start, int lastIndex)
{
    // Randomly Select Different Index for End
    int end = start + ((rand() % (lastIndex)) + 1);

    // Make Sure End Index is Within Range of Array
    if (end >= (lastIndex))
    {
        end = end - lastIndex - 1;
    }
    return end;
}

int main()
{
    // // Generate Directory Name
    // char directoryName[32];
    // pid_t pid = getpid();
    // sprintf(directoryName, "diazh.rooms.%d", pid);

    // // Make Directory
    // makeDirectory(directoryName);

    const int TOTAL_ROOMS = 7;          // Total rooms to generate
    const int TOTAL_ROOM_NAMES = 10;    // Number of Room Names
    const int MAX_ROOM_NAME_CHARS = 8;  // Maximum Number of Characters for Room Name
    const int MIN_CONNECTIONS = 3;      // Minimum Number of Connections for a Room
    const int MAX_CONNECTIONS = 6;      // Maximum Number of Connections for a Room

    // Allocate Memory for All the Rooms
    struct Room* rooms = malloc(TOTAL_ROOMS * sizeof(struct Room));
    // Initialize Number of Connections
    int count;
    for (count = 0; count < TOTAL_ROOMS; count++)
    {
        rooms[count].numConnections = 0;
    }

    // HARD CODE ROOM NAMES
    char roomNames[TOTAL_ROOM_NAMES][MAX_ROOM_NAME_CHARS];
    memset(roomNames, '\0', TOTAL_ROOM_NAMES * MAX_ROOM_NAME_CHARS * sizeof(char));
    strcpy(roomNames[0], "A");  // Room 1
    strcpy(roomNames[1], "B");  // Room 2
    strcpy(roomNames[2], "C");  // Room 3
    strcpy(roomNames[3], "D");  // Room 4
    strcpy(roomNames[4], "E");  // Room 5
    strcpy(roomNames[5], "F");  // Room 6
    strcpy(roomNames[6], "G");  // Room 7
    strcpy(roomNames[7], "H");  // Room 8
    strcpy(roomNames[8], "I");  // Room 9
    strcpy(roomNames[9], "J");  // Room 10

    srand(time(0));     // Seed Random Generator

    // Randomly Select 7 of 10 Hard Coded Room Names.
    int lastIndexNames = TOTAL_ROOM_NAMES - 1;          // Determine Last Index of Rooms Names Array
    for (count = 0; count < TOTAL_ROOMS; count++)
    {
        int randIndex = (rand() % (lastIndexNames + 1));    // Generate Random Index

        // printf("Random Number: %d, Room %d: %s\n", randIndex, count + 1, roomNames[randIndex]); // DEBUGGING

        // Store Name in Rooms Array
        strcpy(rooms[count].name, roomNames[randIndex]);

        // Swap values from randIndex and lastIndex.
        char temp[MAX_ROOM_NAME_CHARS];
        strcpy(temp, roomNames[randIndex]);
        strcpy(roomNames[randIndex], roomNames[lastIndexNames]);
        strcpy(roomNames[lastIndexNames], temp);

        // Make last index inaccessible for next round
        lastIndexNames--;
    }

    // Randomly Determine Indices of Start and End Rooms
    int lastIndexRooms = TOTAL_ROOMS - 1;                           // Last Index of Rooms Array
    int startIndex = (rand() % (lastIndexRooms + 1));               // Randomly Select Index for Start Room
    int endIndex = generateEndIndex(startIndex, lastIndexRooms);    // Randomly Select Index for End Room

    // Set Room Types
    for (count = 0; count < TOTAL_ROOMS; count++)
    {
        if (count == startIndex)
        {
            strcpy(rooms[count].type, "START_ROOM");
        }
        else if (count == endIndex)
        {
            strcpy(rooms[count].type, "END_ROOM");
        }
        else
        {
            strcpy(rooms[count].type, "MID_ROOM");
        }
    }

    // DEBUGGING: View Room Information
    for (count = 0; count < TOTAL_ROOMS; count++)
    {   
        // View Room Names
        printf("==ROOM %d:==\n", count + 1);
        printf("ROOM NAME: %s\n", rooms[count].name);
        printf("NUM CONNECTIONS: %d\n", rooms[count].numConnections);
        printf("ROOM TYPE: %s\n", rooms[count].type);
    }

    free(rooms);

    return 0;
}