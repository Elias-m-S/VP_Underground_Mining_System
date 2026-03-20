###############################################################################
# @file encrypt_file.py
#
# @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
# @date   03.01.2026
#
# @copyright Copyright (c) 2026
#
###############################################################################
#
# @brief Simple script to encrypt a binary file with a simple XOR based
# algorithm. Hereby, the used key for the encryption must be provided a
# commandline parameter
#
#
###############################################################################
import argparse

# Set a default encryption key
DEFAULT_ENCRYPTION_KEY = "VP2026"

# Create an configure the argument parser
argParser = argparse.ArgumentParser(prog='enrypt_file', description='Encrypts a binary file')
argParser.add_argument('filename')
argParser.add_argument('-k', '--key', default=DEFAULT_ENCRYPTION_KEY)
argParser.add_argument('-o', '--output')

# Parse the commandline arguments
args = argParser.parse_args()

# If a valid input file is provided, start encryption
if args.filename:
    # Get the encryption key
    encryptionKey = args.key
    encodedData = []

    # Open the input file
    with open(args.filename, mode='rb') as file:
        # Read the complete file content
        fileContent = file.read()
        # Get a binary representation of the encryption key
        key = str(encryptionKey).encode()
        keyCounter = 0

        # Encrypt each byte of the input file
        for dataByte in fileContent:
            # XOR the input byte with the current key byte
            encodedByte = dataByte ^ key[keyCounter]
            # Add the encrypted byte to the output array
            encodedData.append(encodedByte)

            # Increment the key counter
            keyCounter = keyCounter + 1
            # If the end of the key is reached, perform a rollover of the key counter
            if keyCounter >= len(key):
                keyCounter = 0

    # Define the output file name depending on default settings or commandline parameter
    outputFile = args.filename + ".out"
    if args.output:
        outputFile = args.output

    # Open the output file
    with open(outputFile, mode='wb') as file: # b is important -> binary
        # Get the encrypted data list as byte representation
        dataBytes = bytes(encodedData)
        # Write all byte to the output file
        numBytes = file.write(dataBytes)
        print("{} Bytes written".format(numBytes))
