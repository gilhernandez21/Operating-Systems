import random
import sys

def _getRandomLowChar():
    return chr(random.randrange(ord('a'), ord('z') + 1))

def _getRandomString(numCharacters):
    output = str()
    charCount = 0
    while charCount < numCharacters:
        charCount += 1
        output += str(_getRandomLowChar())
    output += "\n"
    return output

def createRandFile(fileName, numChars):
    fileOut = open(fileName, "w")
    fileOut.write(_getRandomString(numChars))
    fileOut.close()

def getRandomInt(lowerBound, upperBound):
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