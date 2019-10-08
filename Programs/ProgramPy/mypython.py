'''
    File name: mypython.py
    Author: Herbert Diaz (diazh@oregonstate.edu)
    Date created: 10/08/2019
    Last modified: 10/08/2019
    
    Overview: MADE FOR CS 344 - Operating Systems @ Oregon State University
    This script creates 3 files (output1, output2, output3) containing
    a string of 10 randomly generated lowercase letters and a newline character,
    then outputs the contents into stdout. Afterwards, the script randomly
    generates and prints two numbers and their product.
'''

import random
import sys

def _getRandomLowChar():
    ''' Randomly generates and returns a lowercase letter between 'a' and 'z'
        Input: None
        Output: Returns a random character between 'a' and 'z'
    '''
    return chr(random.randrange(ord('a'), ord('z') + 1))

def _getRandomString(numCharacters):
    ''' Creates and returns a string consisting of lowercase letters and a newline
        character
        Input: numCharacters - the number of lowercase letters to generate
        Output: Returns a string of numCharacter letters between 'a' and 'z' and a
                character.
    '''
    output = str()
    charCount = 0
    while charCount < numCharacters:
        charCount += 1
        output += str(_getRandomLowChar())
    output += "\n"
    return output

def createRandFile(fileName, numChars):
    ''' Creates a file with random lowercase letters
        Input:  filename - name of the file
                numChars - the number of lowercase characters to generate
        Output: File with random loewrcase characters generated
    '''
    fileOut = open(fileName, "w")
    fileOut.write(_getRandomString(numChars))
    fileOut.close()

def getRandomInt(lowerBound, upperBound):
    ''' Returns a random number between lowerBound and upperBound
        Input:  lowerBound - the minimum value to return
                upperBound - the maximum value to return
        Output: Returns a random number
    '''
    return random.randint(lowerBound, upperBound)

if __name__ == "__main__":
    NUMCHAR = 10                    # Number of Characters to Put in File
    NUMFILES = 3                    # Number of Files to Make
    LOWERBOUND, UPPERBOUND = 1, 42  # Lower and Upper Bounds for Random Numbers

    # Create Files as "output#" with 10 random characters
    for fileNum in range(NUMFILES):
        fileName = "output" + str(fileNum + 1)
        createRandFile(fileName, NUMCHAR)

    # Output Contents of Output Files
    for fileNum in range(NUMFILES):
        fileName = "output" + str(fileNum + 1)
        with open(fileName, "r") as fileIn:
            sys.stdout.write(fileIn.read())

    # Get Random Integers and Calculate Product
    value1 = getRandomInt(LOWERBOUND, UPPERBOUND)
    value2 = getRandomInt(LOWERBOUND, UPPERBOUND)
    product = value1 * value2

    # Output Values and Product
    sys.stdout.write(str(value1) + "\n")
    sys.stdout.write(str(value2) + "\n")
    sys.stdout.write(str(product) + "\n")