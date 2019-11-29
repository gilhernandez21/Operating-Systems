#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>

#include "otp_helpers.h"
#define h_addr h_addr_list[0]

// File Validation
int checkFile(char* fileName);
void validateFiles(char* ciphertext, char* key);
// Client Function
int sendFile(char* source, char* fileName, char buffer[], char* termString, int socketFD);

int main(int argc, char *argv[])
{
	char* clientVerifier = "OTP_DEC";
	char* terminationString = "$OTP_TERMINATE";
	char* source = "CLIENT";

	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[OTP_BUFFERSIZE];
    
	if (argc < 4) { fprintf(stderr,"USAGE: %s [ciphertext] [key] [port]\n", argv[0]); exit(0); } // Check usage & args

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
	sendMessage(source, clientVerifier, socketFD);
	getResponse(source, buffer, socketFD);
	// If server sends unsuccessful response, print error and exit.
	if (atoi(buffer) != 200)
	{
		fprintf(stderr, "ERROR: Cannot connect to server on port: %d\n", portNumber);
		exit(2);
	}

	// Send ciphertext to server
	sendFile(source, argv[1], buffer, terminationString, socketFD);

	// Send keygen to server
	sendFile(source, argv[2], buffer, terminationString, socketFD);

	// Get plaintext and print to stdout
	while(1)
	{
		getResponse(source, buffer, socketFD);
		sendMessage(source, "200", socketFD);

		// If not termination string, print the buffer
		if (strcmp(buffer, terminationString))
		{
			printf("%s", buffer);
		}
		// Otherwise, exit the loop
		else
		{
			break;
		}
	}

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

void validateFiles(char* ciphertext, char* key)
{
    // Check if files are valid and record number of characters
    int ciphertextCount = checkFile(ciphertext);
    int keyCount = checkFile(key);

    // If the key file is shorter than the ciphertext, terminate and send error
    if (keyCount < ciphertextCount)
    {
        fprintf(stderr, "ERROR key file '%s' shorter than ciphertext '%s'\n", key, ciphertext);
        exit(1);
    }
}

int sendFile(char* source, char* fileName, char buffer[], char* termString, int socketFD)
{

	FILE* fileInput = fopen(fileName, "r"); // Open ciphertext file
	memset(buffer, '\0', OTP_BUFFERSIZE); // Clear out the buffer array

	int count = 0;

	char fileBuffer[OTP_BUFFERSIZE];
	while(fgets(fileBuffer, OTP_BUFFERSIZE, fileInput))
	{
		count++;
		
		// Send message to server
		sendMessage(source, fileBuffer, socketFD);
		// Get return message from server
		getResponse(source, buffer, socketFD);

		memset(buffer, '\0', OTP_BUFFERSIZE); // Clear out the buffer array
	}
	// Send Termination Signal
	sendMessage(source, termString, socketFD);
	getResponse(source, buffer, socketFD);
	// Close File
	fclose(fileInput);
}