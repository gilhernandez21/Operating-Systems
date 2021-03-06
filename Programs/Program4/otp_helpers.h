/*********************************************************************
** Program name:    OTP
** Author:          Herbert Diaz <diazh@oregonstate.edu>
** Date:            12/1/2019
** Description:     Program 4 for CS344 Operating Systems @ OSU
**  Program Function:
**      These are the helper functions for otp_enc, otp_dec,
**      otp_enc_d, and otp_dec_d. These programs encode and decode
**      text using a key. This is the header file.
*********************************************************************/
#ifndef OTP_HELPERS_H
#define OTP_HELPERS_H

#define OTP_BUFFERSIZE 256
#define OTP_MAX_CONNECTIONS 5
#define OTP_NUMCHARS 27

struct OneTimePad {
	char* plaintext;
	char* key;
	char* ciphertext;
};

// Error Functions
void error(const char *msg);
// Send and Recieve Messages
int checkSent(int fileDescriptor);
int sendMessage(char* source, char* message, int fileDescriptor);
int getResponse(char* source, char buffer[], int fileDescriptor);
// Struct OneTimePad Management
int initOTP(struct OneTimePad* pad);
int freeOTP(struct OneTimePad* pad);
// String Manipulation Function
int appendString(char** string, char* input);
// Encoding/Decoding Functions
int getCharVal(char character);
char getIntChar(int value);
int OTP_encode(struct OneTimePad* encoder);
int OTP_decode(struct OneTimePad* decoder);

#endif