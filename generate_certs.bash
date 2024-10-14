#!/bin/bash

###################################################################################################

# Function to check if OpenSSL is installed
check_openssl_installed() {
    if ! command -v openssl &> /dev/null; then
        echo "OpenSSL is not installed. Please install it and try again."
        exit 1
    fi
}

# Function to create directories if they don't exist
create_directories() {
    mkdir -p ./client/data
    mkdir -p ./server/data
    mkdir -p ./server/data/certs
}

###################################################################################################

# Function to create the Certificate Authority (CA)
create_ca() {
    echo "Creating CA certificate..."
    openssl genpkey -algorithm RSA -out ca.key -pkeyopt rsa_keygen_bits:2048
    openssl req -new -x509 -key ca.key -out ca.crt -days 365 -subj "/C=IT/ST=State/L=City/O=MyCompany/OU=IT Department/CN=MyCA"
}

# Function to create the server certificate and its DER versions
create_server_cert() {
    echo "Creating server certificate..."
    openssl genpkey -algorithm RSA -out server.key -pkeyopt rsa_keygen_bits:2048
    openssl req -new -key server.key -out server.csr -subj "/C=IT/ST=State/L=City/O=MyCompany/OU=IT Department/CN=Server"
    openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out server.crt -days 365

    # Convert server certificate and key to DER format
    openssl x509 -in server.crt -outform der -out server.crt.der
    openssl rsa -in server.key -outform der -out server.key.der
}

# Function to create the client certificate
create_client_cert() {
    echo "Creating client certificate..."
    openssl genpkey -algorithm RSA -out client.key -pkeyopt rsa_keygen_bits:2048
    openssl req -new -key client.key -out client.csr -subj "/C=IT/ST=State/L=City/O=MyCompany/OU=IT Department/CN=Client"
    openssl x509 -req -in client.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out client.crt -days 365
}

# Function to move certificates to the correct directories and clean up
transfer_certs() {
    echo "Moving certificates to the correct directories..."

    # Move client certificates to ./client/data/
    cp client.crt client.key ./client/data/

    # Move server certificates in PEM format to ./server/data/certs/
    cp server.crt server.key ./server/data/certs/

    # Move server certificates in DER format to ./server/data/
    cp server.crt.der server.key.der ./server/data/

    # Move CA certificates to both client and server directories
    cp ca.crt ca.key ./client/data/
    cp ca.crt ca.key ./server/data/

    # Clean up: Remove all the no longer needed files
    rm -f *.crt *.csr *.key
}

###################################################################################################

# Main script execution

# Setup environment
check_openssl_installed
create_directories

# Certs Generation
create_ca
create_server_cert
create_client_cert

# Transfer Certs
transfer_certs

echo "Certificate generation and setup complete."
