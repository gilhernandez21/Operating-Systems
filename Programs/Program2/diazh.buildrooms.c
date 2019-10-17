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

int makeDirectory(char* directoryName);
int _generateRoomFile(char* directory, char* file, char* input);
int generateEndIndex(int start, int lastIndex);
void initializeRoomType(struct Room* room, int index, int startIndex, int endIndex);
void printRoom(struct Room room);
char* roomOoutput(struct Room room);

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
                // struct Room* room1 = &rooms[count];
                // int numConnections1 = room1->numConnections;

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

    // DEBUGGING: View Room Information
    for (count = 0; count < TOTAL_ROOMS; count++)
    {
        printf("==ROOM %d:==\n", rooms[count].id + 1);
        printRoom(rooms[count]);
    }

    free(rooms);

    return 0;
}

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

char* roomOutput(struct Room room)
{
    printf("Hello world!");
}