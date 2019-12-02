/*********************************************************************
** Program name:    OTP
** Author:          Herbert Diaz <diazh@oregonstate.edu>
** Date:            12/1/2019
** Description:     Program 4 for CS344 Operating Systems @ OSU
**  Program Function:
**		Usage: otp_enc_d [port]
**		otp_enc_d works with otp_enc to decode a plaintext file
**		into ciphertext, using a provided key. This program serves as
**		the server. This program takes the plaintext and the key from
**		the client, encodes the plaintext, and sends the encoded text
**		back to the client.
**		Code adapted from server.c from Program 4 Lecture
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>

#include "otp_helpers.h"

// Server Functions
int sendVerificationResult(char buffer[], char* clientVerifier, int establishedConnectionFD);
int getClientFile(char* source, char buffer[], char* termString, char** fileString, int establishedConnectionFD);
int sendString(char* output, char buffer[], char* terminationString, int fileDescriptor);

int main(int argc, char *argv[])
{
	char* clientVerifier = "OTP_ENC";
	char* terminationString = "$OTP_TERMINATE";
	char* source = "SERVER";

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

		// Create a fork to get the files, encode the message, the send the ciphertext
		spawnPID = fork();

		switch (spawnPID)
		{
			// Catch errors
			case -1:
			{
				error("fork() failed\n");
				exit(1);
				break;
			}
			// Get the files, encode the message, send the ciphertext
			case 0:
			{
				// Get verifification message from client and send result code back
				getResponse(source, buffer, establishedConnectionFD);
				sendVerificationResult(buffer, clientVerifier, establishedConnectionFD);

				// Initialize the OneTimePad files;
				struct OneTimePad pad;
				initOTP(&pad);

				// Get Plain Text
				getClientFile(source, buffer, terminationString, &pad.plaintext, establishedConnectionFD);

				// Get Key
				getClientFile(source, buffer, terminationString, &pad.key, establishedConnectionFD);

				// Encode Text
				OTP_encode(&pad);

				// Send the cipher text to client
				sendString(pad.ciphertext, buffer, terminationString, establishedConnectionFD);
				
				freeOTP(&pad);					// Clear the One Time Pad
				close(establishedConnectionFD); // Close the existing socket which is connected to the client
				exit(0);
				break;
			}
			// Save the process, check for any completed processes, and continue recieving requests
			default:
			{
				int savedPID = 0;	// Flag checks to see if background PID was saved
				// Add Process to Empty PID
				int index;
				for (index = 0; index < OTP_MAX_CONNECTIONS; index++)
				{
					// Check if Process has Been Completed, if so store background process
					pid_t actualPID = waitpid(backPIDs[index], &childExitMethod, WNOHANG);
					if (actualPID)
					{
						backPIDs[index] = spawnPID;
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

/*********************************************************************
 * int sendVerificationResult(char buffer[], char* clientVerifier, int establishedConnectionFD)
 *  Sends a confirmation that the message was recieved from the client
 * Arguments:
 *  char buffer[] - the buffer that holds the recieved message
 *  char* clientVerifier - the validation code to ensure the usage of
 *  	the correct client.
 * 	int establishedConnectionFD - the fileDescriptor of the connection
 * Returns:
 * 	0 if successful
*********************************************************************/
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
		exit(1);
	}
	return 0;
}

/*********************************************************************
 * int getClientFile(char* source, char buffer[], char* termString, char** fileString, int establishedConnectionFD)
 *  Gets a file from the client and saves as a string
 * Arguments:
 *	char* source - whether the program is a server or client
 *	char buffer[] - holds the message recieved from th eclient
 *	char* termString - the string to indicate that a file has been
 *		completely sent.
 *	char** fileString - the location of the string that will hold the
 *		file sent by the client.
 * 	int establishedConnectionFD - the file descriptor of the connection
 * Returns:
 * 	0 if successful
*********************************************************************/
int getClientFile(char* source, char buffer[], char* termString, char** fileString, int establishedConnectionFD)
{
	int charsRead;

	// Continuously receieve from client, saving to fileString until termination
	// string is recieved from the client.
	while(1)
	{
		// Get part of the file
		getResponse(source, buffer, establishedConnectionFD);

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
			appendString(fileString, "\0");
			break;
		}
	}

	return 0;
}

/*********************************************************************
 * int sendString(char* output, char buffer[], char* terminationString, int fileDescriptor)
 *  Sends a string to the client
 * Arguments:
 *	char* output - the string to be sent to the client
 *	char buffer[] - holds part of the string to send
 *	char* terminationString - the string to indicate that a file has
 *		been completely sent.
 * 	int fileDescriptor - the file descriptor of the connection
 * Returns:
 * 	0 if successful
*********************************************************************/
int sendString(char* output, char buffer[], char* terminationString, int fileDescriptor)
{
    int bufferIndex = 0;
	char* source = "SERVER";

    // Loop Through the String, building a packet to send
    int index;
    for (index = 0; index < strlen(output); index++)
    {
        // If packet reaches 
        if (bufferIndex == OTP_BUFFERSIZE - 1)
        {
			// Store Termination character
			buffer[bufferIndex + 1] = '\0';
			// Reset Iterator
            bufferIndex = 0;
			// Send packet and receive response.
			sendMessage(source, buffer, fileDescriptor);
			getResponse(source, buffer, fileDescriptor);
            memset(buffer, '\0', OTP_BUFFERSIZE);
        }
        buffer[bufferIndex] = output[index];
        bufferIndex++;
    }
    // Send the remaining buffer
	sendMessage(source, buffer, fileDescriptor);
	getResponse(source, buffer, fileDescriptor);
    // Send termination character
	sendMessage(source, terminationString, fileDescriptor);
	getResponse(source, buffer, fileDescriptor);

    return 0;
}