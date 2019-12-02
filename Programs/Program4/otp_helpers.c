/*********************************************************************
** Program name:    OTP
** Author:          Herbert Diaz <diazh@oregonstate.edu>
** Date:            12/1/2019
** Description:     Program 4 for CS344 Operating Systems @ OSU
**  Program Function:
**      These are the helper functions for otp_enc, otp_dec,
**      otp_enc_d, and otp_dec_d. These programs encode and decode
**      text using a key. This is the implementation file.
*********************************************************************/
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

/*********************************************************************
 * int checkSent(int socketFD)
 *  Verifies the message has been sent.
 * Arguments:
 * 	int socketFD - the file descriptor of the connection.
 * Returns:
 * 	0 on success.
*********************************************************************/
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

    return 0;
}

/*********************************************************************
 * int sendMessage(char* source, char* message, int fileDescriptor)
 *  Sends a message to the connection.
 * Arguments:
 * 	char* source - Whether the server or client is sending the message.
 *  char* message - the message to send.
 *  int fileDescriptor - the file descriptor of the connection.
 * Returns:
 * 	0 on success.
*********************************************************************/
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

/*********************************************************************
 * int getResponse(char* source, char buffer[], int fileDescriptor)
 *  Recieves a message from a connection
 * Arguments:
 * 	char* source - Whether the server or client is sending the message.
 *  char* buffer[] - The location to hold the recieves message
 *  int fileDescriptor - the file descriptor of the connection.
 * Returns:
 * 	0 on success.
*********************************************************************/
int getResponse(char* source, char buffer[], int fileDescriptor)
{
	int charsRead;

	memset(buffer, '\0', OTP_BUFFERSIZE); // Clear out the buffer again for reuse
	charsRead = recv(fileDescriptor, buffer, OTP_BUFFERSIZE - 1, 0); // Read data from the socket, leaving \0 at end
	if (charsRead < 0) { fprintf(stderr, "%s", source); error(": ERROR reading from socket"); }
	// printf("%s: I received this from the server: \"%s\"\n", source, buffer); // DEBUGGING

	return 0;
}

/*********************************************************************
 * int getCharVal(char character)
 *  Gets the numerical value of a character
 * Arguments:
 * 	char character - the character to translate
 * Returns:
 * 	int - the int value of the character
 *  -1 if failed
*********************************************************************/
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

/*********************************************************************
 * char getIntChar(int value)
 *  Gets character of the integer value
 * Arguments:
 * 	int value - the integer to translate
 * Returns:
 * 	char - the character of the int value
 *  -1 if failed
*********************************************************************/
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

/*********************************************************************
 * int OTP_encode(struct OneTimePad* encoder)
 *  Encodes a string via the One Time Pad method
 * Arguments:
 * 	struct OneTimePad* encoder - the location of the struct that holds
 *  the plaintext file, the key, and the ciphertext
 * Returns:
 * 	0 on success
*********************************************************************/
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

/*********************************************************************
 * int OTP_decode(struct OneTimePad* encoder)
 *  Decodes a string via the One Time Pad method
 * Arguments:
 * 	struct OneTimePad* decoder - the location of the struct that holds
 *  the plaintext file, the key, and the ciphertext
 * Returns:
 * 	0 on success
*********************************************************************/
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

/*********************************************************************
 * int initOTP(struct OneTimePad* pad)
 *  Initializes the OneTimePad struct
 * Arguments:
 * 	struct OneTimePad* encoder - the location of the struct that holds
 *  the plaintext file, the key, and the ciphertext
 * Returns:
 * 	0 on success
*********************************************************************/
int initOTP(struct OneTimePad* pad)
{
	// Initalize values to NULL
	pad->plaintext = NULL;
	pad->key = NULL;
	pad->ciphertext = NULL;

	return 0;
}

/*********************************************************************
 * int freeOTP(struct OneTimePad* pad)
 *  Frees any allocated memory in the OneTimePad struct
 * Arguments:
 * 	struct OneTimePad* encoder - the location of the struct that holds
 *  the plaintext file, the key, and the ciphertext
 * Returns:
 * 	0 on success
*********************************************************************/
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

    return 0;
}

/*********************************************************************
 * int appendString(char** string, char* input)
 *  Appends a string into another string
 * Arguments:
 * 	char** string - the location of the string to be appended to
 *  char* input - the string to add to the first string
 * Returns:
 * 	0 on success
*********************************************************************/
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