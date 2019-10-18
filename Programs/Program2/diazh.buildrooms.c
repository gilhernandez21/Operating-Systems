/**
 * Program name:    Program 2 - diazh.buildrooms
 * Author:          Herbert Diaz
 * Date created:    10/14/2019
 * Last modified:   10/18/2019
 * Description:
 *      This program randomly generates 7 room files with 3 attributes: the
 *  room name, the room connections, and the type of the room. The room names
 *  are randomly selected from a list of 10 hard-coded room names. The room
 *  connections and room types are also randomly generated.
**/

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
    int id;                         // ID OF THE ROOM
    char name[8];                   // NAME OF THE ROOM
    char type[12];                  // TYPE OF ROOM
    int numConnections;             // NUMBER OF CONNECTIONS TO ROOM
    struct Room* connections[6];    // THE CONNECTED ROOMS
};

// Prototypes
int makeDirectory(char* directoryName);                             // Creates a new directory
int generateRoomFile(char* directory, char* file, char* input);     // Creates a new room file
int generateEndIndex(int start, int lastIndex);                     // Determines an ending index
void initializeRoomType(struct Room* room, int index, int startIndex, int endIndex);    // Sets Room Type
void printRoom(struct Room room);                                   // Prints room details to stdout

int main()
{
    // Generate Directory Name
    char directoryName[32];
    pid_t pid = getpid();
    sprintf(directoryName, "diazh.rooms.%d", pid);

    // Make Directory
    makeDirectory(directoryName);

    const int TOTAL_ROOMS = 7;          // Total rooms to generate
    const int TOTAL_ROOM_NAMES = 10;    // Number of Room Names
    const int MAX_ROOM_NAME_CHARS = 8;  // Maximum Number of Characters for Room Name
    const int MIN_CONNECTIONS = 3;      // Minimum Number of Connections for a Room
    const int MAX_CONNECTIONS = 6;      // Maximum Number of Connections for a Room

    // Allocate Memory for All the Rooms
    struct Room* rooms = malloc(TOTAL_ROOMS * sizeof(struct Room));
    // Initialize ID and Number of Connections
    int count;
    for (count = 0; count < TOTAL_ROOMS; count++)
    {
        rooms[count].id = count;
        rooms[count].numConnections = 0;
    }

    // HARD CODE ROOM NAMES
    char roomNames[TOTAL_ROOM_NAMES][MAX_ROOM_NAME_CHARS];
    memset(roomNames, '\0', TOTAL_ROOM_NAMES * MAX_ROOM_NAME_CHARS * sizeof(char));
    strcpy(roomNames[0], "Alley");      // Room 1
    strcpy(roomNames[1], "Bar");        // Room 2
    strcpy(roomNames[2], "Church");     // Room 3
    strcpy(roomNames[3], "Dungeon");    // Room 4
    strcpy(roomNames[4], "Embassy");    // Room 5
    strcpy(roomNames[5], "Forest");     // Room 6
    strcpy(roomNames[6], "Guild");      // Room 7
    strcpy(roomNames[7], "House");      // Room 8
    strcpy(roomNames[8], "Imps");       // Room 9
    strcpy(roomNames[9], "Jail");       // Room 10

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
        initializeRoomType(&rooms[count], count, startIndex, endIndex);
    }

    // Create Connections
    for (count = 0; count < TOTAL_ROOMS; count++)
    {
        struct Room* room1 = &rooms[count];
        int numConnections1 = room1->numConnections;
        while (numConnections1 < MIN_CONNECTIONS)
        {
            int randRoom = (rand() % TOTAL_ROOMS);  // pick random room

            // Make sure random room is not itself
            if (randRoom != count)
            {
                // Check to see if random room is not already connected
                int isValid = 1;
                int index;
                for (index = 0; index < numConnections1; index++)
                {
                    struct Room* tempRoom = room1->connections[index];
                    if (randRoom == tempRoom->id)
                    {
                        isValid = 0;
                        break;
                    }
                }
                
                // Connect random room
                if (isValid == 1)
                {
                    struct Room* room2 = &rooms[randRoom];
                    int numConnections2 = room2->numConnections;

                    room1->connections[numConnections1] = room2;
                    room1->numConnections += 1;
                    room2->connections[numConnections2] = room1;
                    room2->numConnections += 1;

                    numConnections1 = room1->numConnections;
                }
            }
        }
    }

    // Write All Rooms into Files
    for(count = 0; count < TOTAL_ROOMS; count++)
    {
        const int lineLength = 30;

        // Generate File Name
        char fileName[lineLength];
        memset(fileName, '\0', lineLength * sizeof(char));
        sprintf(fileName, "%s.room", rooms[count].name);

        // Create Name Line
        char nameLine[lineLength];
        memset(nameLine, '\0', lineLength * sizeof(char));
        sprintf(nameLine, "ROOM NAME: %s\n", rooms[count].name);
        generateRoomFile(directoryName, fileName, nameLine);

        // Create Connection Lines
        int index;
        for (index = 0; index < rooms[count].numConnections; index++)
        {
            struct Room* tempRoom = (rooms[count].connections)[index];
            char* tempRoomName = tempRoom->name;
            char connectionLine[lineLength];
            memset(connectionLine, '\0', lineLength * sizeof(char));
            sprintf(connectionLine, "CONNECTION %d: %s\n", index + 1, tempRoomName);
            generateRoomFile(directoryName, fileName, connectionLine);
        }

        // Create Type Line
        char typeLine[lineLength];
        memset(typeLine, '\0', lineLength * sizeof(char));
        sprintf(typeLine, "ROOM TYPE: %s\n", rooms[count].type);
        generateRoomFile(directoryName, fileName, typeLine);
    }

    free(rooms);

    return 0;
}

/**
 * makeDirectory(char*)
 * Input
 *  char* directoryName - the name of the directory to be made
 * Output
 *  int - 0 if successful, otherwise exits with 1;
 * Description
 *  This function creates a directory
**/
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

/**
 * generateRoomFile(char* directory, char* file, char* input)
 * Input
 *  char* directory - the directory to place the file into
 *  char* file - the name of the file to be created
 *  char* input - the data to write to the file
 * Output
 *  int - returns 0 on success, otherwise returns 2 and exits
 * Description
 *  creates a room file into a selected directory
**/
int generateRoomFile(char* directory, char* file, char* input)
{
    // generate location of file
    char location[64];
    sprintf(location, "./%s/%s", directory, file);

    // open file
    int fileDescriptor = open(location, O_WRONLY | O_APPEND | O_CREAT, 0600);

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

/**
 * generateEndIndex(int start, int lastIndex)
 * Input
 *  int start - the current index
 *  int lastindex - the last possible index of the array
 * Output
 *  int - the value where the end index would be
 * Description
 *  This function randomly determines where the end index will be.
**/
int generateEndIndex(int start, int lastIndex)
{
    // Randomly Select Different Index for End
    int random = (rand() % lastIndex) + 1;
    int end = start + random;
    
    // Make Sure End Index is Within Range of Array
    if (end > lastIndex)
    {
        end = end - lastIndex - 1;
    }

    return end;
}

/**
 * initializeRoomType(sturct Room* room, int index, int startIndex, int endindex)
 * Input
 *  struct Room* room - pointer to a room struct to modify
 *  int index - the current index
 *  int startIndex - location of the starting index
 *  int endINdex - location of the ending index
 * Description
 *  Sets a Room's type to either START_INDEX, END_INDEX, or MID_INDEX
**/
void initializeRoomType(struct Room* room, int index, int startIndex, int endIndex)
{
    if (index == startIndex)
    {
        strcpy(room->type, "START_ROOM");
    }
    else if (index == endIndex)
    {
        strcpy(room->type, "END_ROOM");
    }
    else
    {
        strcpy(room->type, "MID_ROOM");
    }
}

/**
 * printRoom
 * Input
 *  struct Room room - the room to be printed
 * Output
 *  Sends Room struct's data into stdout
 * Description
 *  Prints all relevant data of a Room struct.
**/
void printRoom(struct Room room)
{
    printf("ROOM NAME: %s\n", room.name);
    // printf("NUM CONNECTIONS: %d\n", room.numConnections);   // DEBUGGING
    int count;
    for (count = 0; count < room.numConnections; count++)
    {
        struct Room* tempRoom = room.connections[count];
        char* roomName = tempRoom->name;
        printf("CONNECTION %d: %s\n", count + 1, roomName);
    }
    printf("ROOM TYPE: %s\n", room.type);
}