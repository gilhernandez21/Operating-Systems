#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>

#define h_addr h_addr_list[0]
#define OTP_BUFFERSIZE 256

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

// File Validation
int checkFile(char* fileName);
void validateFiles(char* plaintext, char* key);

int sendMessage(char buffer[], char* message, int socketFD);
int getResponse(char buffer[], int socketFD);

int main(int argc, char *argv[])
{
	char* clientVerifier = "OTP_ENC";
	char* terminationString = "$OTP_TERMINATE";

	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[OTP_BUFFERSIZE];
    
	if (argc < 4) { fprintf(stderr,"USAGE: %s [plaintext] [key] [port]\n", argv[0]); exit(0); } // Check usage & args

	// Check files for bad characters and proper lengths
	validateFiles(argv[1], argv[2]);

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");

	// Send verifier to server and get response
	sendMessage(buffer, clientVerifier, socketFD);
	getResponse(buffer, socketFD);
	// If server sends unsuccessful response, print error and exit.
	if (atoi(buffer) != 200)
	{
		fprintf(stderr, "ERROR: Cannot connect to server on port: %d\n", portNumber);
		exit(2);
	}

	// Send plaintext to server
	FILE* fileInput = fopen(argv[1], "r"); // Open plaintext file
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array

	char fileBuffer[OTP_BUFFERSIZE];
    while(fgets(fileBuffer, sizeof(fileBuffer), fileInput))
    {
		// Send message to server
		sendMessage(buffer, fileBuffer, socketFD);
		// Get return message from server
		getResponse(buffer, socketFD);

        memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array
    }
	fclose(fileInput);
	sendMessage(buffer, terminationString, socketFD);
	getResponse(buffer, socketFD);

	// Send keygen to server
	fileInput = fopen(argv[2], "r");
	fclose(fileInput);

	// Get ciphertext and print to stdout

	close(socketFD); // Close the socket
	return 0;
}

int checkFile(char* fileName)
{
    int character;  // holds the integer value of the character
    int count = 0;  // holds the number of characters in the file

    // Open the file
    FILE* fileInput = fopen(fileName, "r");
    // Check if valid
    if (fileInput == NULL) { fprintf(stderr,"ERROR failed to open '%s'\n", fileName); exit(1); }

    // Go through the file
    while ((character = fgetc(fileInput)) != EOF)
    {
        // If an invalid character is detected, print error and exit
        if ((character < 'A' || character > 'Z') && character != ' ' && character != '\n')
        {
            fprintf(stderr,"ERROR '%s' contains invalid characters\n", fileName);
            exit(1);
        }
        count++;
    }

    // Close the file
    fclose(fileInput);

    return count;
}

void validateFiles(char* plaintext, char* key)
{
    // Check if files are valid and record number of characters
    int plaintextCount = checkFile(plaintext);
    int keyCount = checkFile(key);

    // If the key file is shorter than the plaintext, terminate and send error
    if (keyCount < plaintextCount)
    {
        fprintf(stderr, "ERROR key file '%s' shorter than plaintext '%s'\n", key, plaintext);
        exit(1);
    }
}

int sendMessage(char buffer[], char* message, int socketFD)
{
	int charsWritten;

	memset(buffer, '\0', OTP_BUFFERSIZE); 	// Clear out the buffer array
	strcpy(buffer, message);			// Set verifier
	charsWritten = send(socketFD, buffer, strlen(buffer), 0); // Write to the server
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");
	
	return 0;
}

int getResponse(char buffer[], int socketFD)
{
	int charsRead;

	memset(buffer, '\0', OTP_BUFFERSIZE); // Clear out the buffer again for reuse
	charsRead = recv(socketFD, buffer, OTP_BUFFERSIZE - 1, 0); // Read data from the socket, leaving \0 at end
	if (charsRead < 0) error("CLIENT: ERROR reading from socket");
	// printf("CLIENT: I received this from the server: \"%s\"\n", buffer); // DEBUGGING

	return 0;
}