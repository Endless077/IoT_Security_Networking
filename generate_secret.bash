#!/bin/bash

# Ensure client and server data directories exist
mkdir -p ./client/data/
mkdir -p ./server/data/

# Prompt the user for a secret message
echo "Enter a secret message for the secret.txt file:"
read -r secret_message

# Create secrets.txt with the user-defined message
echo "$secret_message" > secret.txt

# Move secrets.txt to client and server data directories
cp secret.txt ./client/data/
cp secret.txt ./server/data/

# Clean up secrets.txt
rm -f secret.txt

echo "secret.txt created and copied to client and server directories."
