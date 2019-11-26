#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define OTP_BUFFERSIZE 256

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int getFromClient(char buffer[], int establishedConnectionFD);
int sendVerificationResult(char buffer[], char* clientVerifier, int establishedConnectionFD);

int main(int argc, char *argv[])
{
	char* clientVerifier = "OTP_ENC";

	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	char buffer[OTP_BUFFERSIZE];
	struct sockaddr_in serverAddress, clientAddress;

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
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	do
	{
		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) error("ERROR on accept");

		// Get Verifification Message from Client
		getFromClient(buffer, establishedConnectionFD);
		// Send result code back to the client
		sendVerificationResult(buffer, clientVerifier, establishedConnectionFD);

		// Get Plaintext

		// // Get the message from the client and display it
		// memset(buffer, '\0', OTP_BUFFERSIZE);
		// charsRead = recv(establishedConnectionFD, buffer, OTP_BUFFERSIZE - 1, 0); // Read the client's message from the socket
		// if (charsRead < 0) error("ERROR reading from socket");
		// printf("SERVER: I received this from the client: \"%s\"\n", buffer);

		// // Send a Success message back to the client
		// charsRead = send(establishedConnectionFD, "I am the server, and I got your message", 39, 0); // Send success back
		// if (charsRead < 0) error("ERROR writing to socket");

		// Get Key File

		close(establishedConnectionFD); // Close the existing socket which is connected to the client
	} while(1);

	close(listenSocketFD); // Close the listening socket
	return 0; 
}

int getFromClient(char buffer[], int establishedConnectionFD)
{
	int charsRead;

	memset(buffer, '\0', OTP_BUFFERSIZE);
	charsRead = recv(establishedConnectionFD, buffer, OTP_BUFFERSIZE - 1, 0); // Read the client's message from the socket
	if (charsRead < 0) error("ERROR reading from socket");
	printf("SERVER: I received this from the client: \"%s\"\n", buffer); // DEBUGGING

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