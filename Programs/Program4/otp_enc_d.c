#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>

#define OTP_BUFFERSIZE 256
#define OTP_MAX_CONNECTIONS 5
#define OTP_NUMCHARS 26

struct OneTimePad {
	char* plaintext;
	char* key;
	char* ciphertext;
};

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int getFromClient(char buffer[], int establishedConnectionFD);
int sendVerificationResult(char buffer[], char* clientVerifier, int establishedConnectionFD);
int sendMessage(char* message, int socketFD);

int initOTP(struct OneTimePad* pad);
int freeOTP(struct OneTimePad* pad);
int appendString(char** string, char* input);
int getClientFile(char buffer[], char* termString, char** fileString, int establishedConnectionFD);

int getCharVal(char character)
{
    if (character >= 'A' && character <= 'Z')
    {
        return character - 'A';
    }
    else if (character == ' ')
    {
        return 26;
    }
    else
    {
        fprintf(stderr, "ERROR invalid character '%c' detected\n", character);
        exit(1);
    }
    return -1;
}

char getIntChar(int value)
{
    if (value == OTP_NUMCHARS) {return ' ';}
    else if (value < OTP_NUMCHARS) {return (char) value + 'A';}
    else
    {
        fprintf(stderr, "ERROR invalid value '%d' detected\n", value);
        exit(1);
    }
    return -1;
}
    
int OTP_decode(struct OneTimePad* decoder)
{
    int ciphertextLength = strlen(decoder->ciphertext), keyLength = strlen(decoder->key);
    if (ciphertextLength > keyLength) {error("ERROR decoder->ciphertext length greater than decoder->key length\n");}
    char ciphertextCharacter, keyCharacter;
    decoder->plaintext = malloc(ciphertextLength * sizeof(char));

    int index;
    for(index = 0; index < ciphertextLength - 1; index++)
    {
        int messageKey = getCharVal(decoder->ciphertext[index]) - getCharVal(decoder->key[index]);
        char plainChar = getIntChar((messageKey + OTP_NUMCHARS) % OTP_NUMCHARS);
        decoder->plaintext[index] = plainChar;
    }
    decoder->plaintext[index] = '\n';

    return 0;
}

int OTP_decode(char* ciphertext, char* key, char** plaintext)
{
    int ciphertextLength = strlen(ciphertext), keyLength = strlen(key);
    if (ciphertextLength > keyLength) {error("ERROR ciphertext length greater than key length\n");}
    char ciphertextCharacter, keyCharacter;
    *plaintext = malloc(ciphertextLength * sizeof(char));

    int index;
    for(index = 0; index < ciphertextLength - 1; index++)
    {
        int messageKey = getCharVal(ciphertext[index]) - getCharVal(key[index]);
        char plainChar = getIntChar((messageKey + OTP_NUMCHARS) % OTP_NUMCHARS);
        (*plaintext)[index] = plainChar;
    }
    (*plaintext)[index] = '\n';

    return 0;
}

int main(int argc, char *argv[])
{
	char* clientVerifier = "OTP_ENC";
	char* terminationString = "$OTP_TERMINATE";

	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	char buffer[OTP_BUFFERSIZE];
	struct sockaddr_in serverAddress, clientAddress;

	pid_t backPIDs[OTP_MAX_CONNECTIONS];
	int index;
	for (index = 0; index < OTP_MAX_CONNECTIONS; index++)
	{
		backPIDs[index] = 1;
	}

	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, OTP_MAX_CONNECTIONS); // Flip the socket on - it can now receive up to 5 connections

	do
	{
		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) error("ERROR on accept");

		pid_t spawnPID = -5;
		int childExitMethod = -5;

		spawnPID = fork();

		switch (spawnPID)
		{
			case -1:
			{
				error("fork() failed\n");
				exit(1);
				break;
			}
			case 0:
			{
				// Get verifification message from client and send result code back
				getFromClient(buffer, establishedConnectionFD);
				sendVerificationResult(buffer, clientVerifier, establishedConnectionFD);

				// Initialize the OneTimePad files;
				struct OneTimePad pad;
				initOTP(&pad);

				// Get Plain Text
				getClientFile(buffer, terminationString, &pad.plaintext, establishedConnectionFD);

				// Get Key
				getClientFile(buffer, terminationString, &pad.key, establishedConnectionFD);

				// Encode Text
				

				// Send the cipher text to client
				// sendMessage(pad.plaintext, establishedConnectionFD);

				// printf("Plain Text:\n%s\n", pad.plaintext);
				// printf("Key:\n%s\n", pad.key);

				freeOTP(&pad);

				close(establishedConnectionFD); // Close the existing socket which is connected to the client
				exit(0);
				break;
			}
			default:
			{
				int savedPID = 0;	// Flag checks to see if background PID was saved
				// Add Process to Empty PID
				int index;
				for (index = 0; index < OTP_MAX_CONNECTIONS; index++)
				{
					// Check if Process has Been Completed, if so store background process
					pid_t actualPID = waitpid(backPIDs[index], &childExitMethod, WNOHANG);
					// printf("Actual PID: %d, Background PID: %d\n", actualPID, backPIDs[index]); // DEBUGGING
					if (actualPID)
					{
						backPIDs[index] = spawnPID;
						// printf("Stored PID: %d\n", backPIDs[index]); DEBUGGING
						savedPID = 1;
						break;
					}
				}
				// If process ID couldn't be saved, print error and retrieve child
				if (!savedPID)
				{
					fprintf(stderr, "ERROR: Processes exceed max connections, retriveing child...\n");
					waitpid(spawnPID, &childExitMethod, 0);
				}
				break;
			}
		}

		// Wait Children
		int index;
		for (index = 0; index < OTP_MAX_CONNECTIONS; index++)
		{
			// Check if Process has Been Completed
			pid_t actualPID = waitpid(backPIDs[index], &childExitMethod, WNOHANG);
		}

	} while(1);

	close(listenSocketFD); // Close the listening socket

	// catch all remaining children
	int childExitMethod = -5;
	for (index = 0; index < OTP_MAX_CONNECTIONS; index++)
	{
		// Check if Process has Been Completed
		pid_t actualPID = waitpid(backPIDs[index], &childExitMethod, 0);
	}
	return 0; 
}

int getFromClient(char buffer[], int establishedConnectionFD)
{
	int charsRead;

	memset(buffer, '\0', OTP_BUFFERSIZE);
	charsRead = recv(establishedConnectionFD, buffer, OTP_BUFFERSIZE - 1, 0); // Read the client's message from the socket
	if (charsRead < 0) error("ERROR reading from socket");
	// printf("SERVER: I received this from the client: \"%s\"\n", buffer); // DEBUGGING

	return 0;
}

int sendVerificationResult(char buffer[], char* clientVerifier, int establishedConnectionFD)
{
	int charsRead;

	if (!strcmp(buffer, clientVerifier))
	{
		charsRead = send(establishedConnectionFD, "200", 3, 0); // Send success back
		if (charsRead < 0) error("ERROR writing to socket");
	}
	else
	{
		charsRead = send(establishedConnectionFD, "403", 3, 0); // Send failure back
		if (charsRead < 0) error("ERROR writing to socket");
		close(establishedConnectionFD); // Close the existing socket which is connected to the client
		return 1;
	}
	return 0;
}

int initOTP(struct OneTimePad* pad)
{
	// Initalize values to NULL
	pad->plaintext = NULL;
	pad->key = NULL;
	pad->ciphertext = NULL;

	return 0;
}

int freeOTP(struct OneTimePad* pad)
{
	if (pad->plaintext != NULL)
	{
		free(pad->plaintext);
	}
	if (pad->key != NULL)
	{
	free(pad->key);
	}
	if (pad->ciphertext != NULL)
	{
		free(pad->ciphertext);
	}
}

int appendString(char** string, char* input)
{
    // If the string is not empty, combine string and input
    if (*string != NULL)
    {
        // Make a temporary string
        size_t newLength = strlen(*string) + strlen(input) + 1;
        char* temp = malloc(newLength * sizeof(char));
        // Combine old string and input into temporary string
        sprintf(temp, "%s%s", *string, input);
        // Save new string
        free(*string);
        *string = malloc(newLength * sizeof(char));
        strcpy(*string, temp);
        // Deallocate Temp
        free(temp);
    }
    // Otherwise, initialize the empty string and set it to the input;
    else
    {
        *string = malloc(strlen(input) * sizeof(char));
        strcpy(*string, input);
    }

    return 0;
}

int getClientFile(char buffer[], char* termString, char** fileString, int establishedConnectionFD)
{
	int charsRead;

	// Continuously receieve from client, saving to fileString until termination
	// string is recieved from the client.
	while(1)
	{
		// Get part of the file
		getFromClient(buffer, establishedConnectionFD);

		// Send confirmation that message was recieved to client
		charsRead = send(establishedConnectionFD, "200", 3, 0); // Send success back
		if (charsRead < 0) error("ERROR writing to socket");

		// If termination string wasn't recieved, save string
		if (strcmp(buffer, termString))
		{
			appendString(fileString, buffer);
		}
		// Otherwise, exit the loop
		else
		{
			break;
		}
	}

	return 0;
}

int sendMessage(char* message, int connectionFD)
{
	int charsWritten;

	// printf("SERVER: I sent this to the client \"%s\" %ld\n", message, strlen(message));
	charsWritten = send(connectionFD, message, strlen(message), 0); // Write to the server
	if (charsWritten < 0) error("SERVER: ERROR writing to socket");
	if (charsWritten < strlen(message)) printf("SERVER: WARNING: Not all data written to socket!\n");

	return 0;
}