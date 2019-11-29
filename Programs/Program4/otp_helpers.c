#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/wait.h>

#include "otp_helpers.h"

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int checkSent(int socketFD)
{
	int checkSend = 5;
	do
	{
		ioctl(socketFD, TIOCOUTQ, &checkSend);
	} while (checkSend > 0);
	if (checkSend < 0)
	{
		error("ioctl error");
	}
}

int sendMessage(char* source, char* message, int fileDescriptor)
{
	int charsWritten;

	// printf("%s: I sent this to the client \"%s\" %ld\n", source, message, strlen(message));
	charsWritten = send(fileDescriptor, message, strlen(message), 0); // Write to the server
	if (charsWritten < 0) { fprintf(stderr, "%s", source); error(": ERROR writing to socket"); }
	if (charsWritten < strlen(message)) printf("%s: WARNING: Not all data written to socket!\n", source);
	checkSent(fileDescriptor);
	
	return 0;
}

int getResponse(char* source, char buffer[], int fileDescriptor)
{
	int charsRead;

	memset(buffer, '\0', OTP_BUFFERSIZE); // Clear out the buffer again for reuse
	charsRead = recv(fileDescriptor, buffer, OTP_BUFFERSIZE - 1, 0); // Read data from the socket, leaving \0 at end
	if (charsRead < 0) { fprintf(stderr, "%s", source); error(": ERROR reading from socket"); }
	// printf("%s: I received this from the server: \"%s\"\n", source, buffer); // DEBUGGING

	return 0;
}

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
    if (value == OTP_NUMCHARS - 1) {return ' ';}
    else if (value < OTP_NUMCHARS) {return (char) value + 'A';}
    else
    {
        fprintf(stderr, "ERROR invalid value '%d' detected\n", value);
        exit(1);
    }
    return -1;
}
    
int OTP_encode(struct OneTimePad* encoder)
{
    int plaintextLength = strlen(encoder->plaintext), keyLength = strlen(encoder->key);
    if (plaintextLength > keyLength) {error("ERROR plaintext length greater than key length\n");}
    char plaintextCharacter, keyCharacter;
    encoder->ciphertext = malloc((plaintextLength + 1) * sizeof(char));

    int index;
    for(index = 0; index < plaintextLength - 1; index++)
    {
        int messageKey = getCharVal(encoder->plaintext[index]) + getCharVal(encoder->key[index]);
        char cipherChar = getIntChar(messageKey % OTP_NUMCHARS);
        (encoder->ciphertext)[index] = cipherChar;
    }
    (encoder->ciphertext)[index] = '\n';
    (encoder->ciphertext)[index+1] = '\0';

    return 0;
}

int OTP_decode(struct OneTimePad* decoder)
{
    int ciphertextLength = strlen(decoder->ciphertext), keyLength = strlen(decoder->key);
    if (ciphertextLength > keyLength) {error("ERROR ciphertext length greater than key length\n");}
    char ciphertextCharacter, keyCharacter;
    decoder->plaintext = malloc((ciphertextLength + 1) * sizeof(char));

    int index;
    for(index = 0; index < ciphertextLength - 1; index++)
    {
        int messageKey = getCharVal(decoder->ciphertext[index]) - getCharVal(decoder->key[index]);
        char plainChar = getIntChar((messageKey + OTP_NUMCHARS) % OTP_NUMCHARS);
        decoder->plaintext[index] = plainChar;
    }
    (decoder->plaintext)[index] = '\n';
    (decoder->plaintext)[index+1] = '\0';

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