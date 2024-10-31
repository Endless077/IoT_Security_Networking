#!/bin/bash

###################################################################################################
# Check if OpenSSL is installed
if ! command -v openssl &> /dev/null; then
    echo "OpenSSL is not installed. Please install it and try again."
    exit 1
fi

# Create necessary directories for server and client
mkdir -p ./client/data/certs
mkdir -p ./server/data/certs

###################################################################################################
# Create Certificate Authority (CA)

echo "Creating CA certificate..."
openssl genpkey -algorithm RSA -out ca_key.pem -pkeyopt rsa_keygen_bits:2048
openssl req -x509 -new -key ca_key.pem -sha256 -days 3650 -out ca_cert.pem -subj "/C=IT/ST=Campania/L=Napoli/O=IoTSecurity/OU=Security/CN=ESP32CA"

# Convert Certification Authority certificate and ket to DER format
openssl x509 -inform PEM -outform DER -in ca_cert.pem -out ca_cert.der
openssl rsa -inform PEM -outform DER -in ca_key.pem -out ca_key.der

###################################################################################################
# Create Server Certificate

echo "Creating server certificate..."
openssl genpkey -algorithm RSA -out server_key.pem -pkeyopt rsa_keygen_bits:2048
openssl req -new -key server_key.pem -out server_csr.pem -subj "/C=IT/ST=Campania/L=Naples/O=IoTSec/OU=Security/CN=esp32server.local"
openssl x509 -req -in server_csr.pem -CA ca_cert.pem -CAkey ca_key.pem -CAcreateserial -out server_cert.pem -days 3650 -sha256

# Convert server certificate and key to DER format
openssl x509 -inform PEM -outform DER -in server_cert.pem -out server_cert.der
openssl rsa -inform PEM -outform DER -in server_key.pem -out server_key.der

###################################################################################################
# Create Client Certificate

echo "Creating client certificate..."
openssl genpkey -algorithm RSA -out client_key.pem -pkeyopt rsa_keygen_bits:2048
openssl req -new -key client_key.pem -out client_csr.pem -subj "/C=IT/ST=Campania/L=Naples/O=IoTSec/OU=Security/CN=esp32client.local"
openssl x509 -req -in client_csr.pem -CA ca_cert.pem -CAkey ca_key.pem -CAcreateserial -out client_cert.pem -days 3650 -sha256

# Convert client certificate and key to DER format
openssl x509 -inform PEM -outform DER -in client_cert.pem -out client_cert.der
openssl rsa -inform PEM -outform DER -in client_key.pem -out client_key.der

###################################################################################################
# Move files to appropriate directories

echo "Moving certificates and keys to the correct directories..."

# Client certificates and keys
cp client_cert.pem client_key.pem ./client/data/certs
cp client_cert.der client_key.der ./client/data/

# Server certificates and keys
cp server_cert.pem server_key.pem ./server/data/certs/
cp server_cert.der server_key.der ./server/data/

# CA certificate and key
cp ca_cert.pem ./client/data/certs/
cp ca_cert.der ./client/data/

cp ca_cert.pem ./server/data/certs/
cp ca_cert.der ./server/data/

echo "Moving certificates and keys to the correct directories complete."

###################################################################################################
# Clean up temporary files

echo "Cleaning up temporary files and organizing backups..."

# Create backup directories if they don't exist
mkdir -p backup/certs

# Move .pem files to backup/certs
mv *.pem backup/certs/

# Move .der files to backup
mv *.der backup/

# Remove only temporary files
rm -f *.srl *.csr

echo "Cleaning up temporary files and organizing backups complete."

###################################################################################################
# Done

echo "Done."
