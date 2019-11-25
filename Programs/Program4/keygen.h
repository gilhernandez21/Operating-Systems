/****************************************************************
 * Program name:    keygen
 * Author:          Herbert Diaz <diazh@oregonstate.edu>
 * Date:            11/24/2019
 * Description:     Program 4 for CS344 Operating Systems @ OSU
 *  Program Function:
 *      This program generates a key consisting of 27 possible characters.
 *  Arguments:
 *      The length of the key
 *  Returns:
 *      Prints the key.
****************************************************************/
#ifndef KEYGEN_H
#define KEYGEN_H

void checkArgCount(int numArgs);
int getKeyLength(char* input);
int _pickRandInt(int min, int max);
char pickRandChar();
void printKey(int numChar);

#endif