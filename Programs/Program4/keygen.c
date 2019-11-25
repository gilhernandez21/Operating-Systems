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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "keygen.h"

int main(int argc, char* argv[])
{
    int keyLength;  // The Length of the Key File
    srand(time(0)); // Seed Randomizer

    checkArgCount(argc); // Check for Valid Number of Arguments
    keyLength = getKeyLength(argv[1]); // Save the Key Length

    printKey(keyLength); // Output the Randomly Generated Key

    return 0;
}

/****************************************************************
 * void checkArgCount(int numArgs)
 *  Shows usage if there are no additional arguments. Otherwise,
 *  prints an error.
 * Arguments:
 *  int numArgs = the number of arguments
****************************************************************/
void checkArgCount(int numArgs)
{
    // If no arguments, print how to use the program
    if (numArgs < 2)
    {
        fprintf(stderr, "Usage: keygen [keyLength]\n");
        exit(1);
    }
    // If more than 2 arguments, inform that there are too many arguments
    else if (numArgs > 2)
    {
        fprintf(stderr, "Error: Too Many Arguments\n");
        exit(1);
    }
}

/****************************************************************
 * int getKeyLength(char* input)
 *  Converts the input into an integer. If the value is less than
 *  one, or not an integer, the program prints an error.
 * Arguments:
 *  char* input = the string containing the key length argument
 * Returns:
 *  int = the length the key
****************************************************************/
int getKeyLength(char* input)
{
    int keyLength = atoi(input);

    // If key length invalid, print error and exit.
    if (keyLength  < 1)
    {
        fprintf(stderr, "ERROR: '%s' is an Invalid Key Length.\n", input);
        exit(1);
    }

    // Otherwise, return the valid keyLength;
    return keyLength;
}

/****************************************************************
 * int _pickRandInt(int min, int max)
 *  Returns a random number between min and max, inclusively.
 * Arguments:
 *  int min = the minimum possible value to choose
 *  int max = the maximum possible value to choose
 * Returns:
 *  int = random number between
****************************************************************/
int _pickRandInt(int min, int max)
{
    return (rand() % (max - min + 1)) + min;
}

/****************************************************************
 * char pickRandChar()
 *  Randomly picks from the choice of a letter from A-Z or a
 *  space character
 * Returns:
 *  char = Randomly determined A-Z, or space
****************************************************************/
char pickRandChar()
{
    char randChar = ' '; // Set a random character to a 

    // Pick a random integer between 0 and 26
    int value = _pickRandInt(0, 26);

    // If the Value is not 26, set random character to a letter
    if (value < 26)
    {
        randChar = (char) (65 + value);
    }
    return randChar;
}

/****************************************************************
 * void printKey(int numChar)
 *  Prints the randomly generated key into stdout.
 * Arguments:
 *  int numChar = the length of the key to generate
****************************************************************/
void printKey(int numChar)
{
    // Print a random character numChar times.
    int count;
    for (count = 0; count < numChar; count++)
    {
        printf("%c", pickRandChar());
    }
    // Print Newline
    printf("\n");
}