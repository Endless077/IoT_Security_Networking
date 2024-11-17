![Logo](https://github.com/Endless077/IoT-Security-Networking/blob/main/espressif_logo.svg)

# IoT Security Networking 🌐🔐

The *IoT Security Networking* project implements a secure client-server communication system between two ESP32 modules, supporting both HTTP and HTTPS protocols. Designed for IoT networking and security environments, this project facilitates secure data exchange with advanced protection options, including automated certificate and secret generation scripts.


## 🔑 Key Features

- **🔗 Flexible Connectivity**: The project can be configured to work with both standard HTTP and HTTPS protocols for encrypted connections.
- **🔧 Automated Certificate Generation**: Includes bash scripts to generate the necessary certificates and secrets for secure connections.
- **📡 IoT Networking Compatibility**: Optimized for IoT devices, using ESP32 to manage communication with advanced networking features.
- **🔒 Secure Communication**: Supports HTTP and HTTPS communication between two ESP32 devices, ensuring secure and protected data transmission.


## 🛠️ Installation

The installation requires PlatformIO (or any other IDE, such as Arduino IDE) for project management and code compilation, along with basic knowledge of Arduino and ESP-IDF to configure the ESP32 modules correctly.

### 🔧 Environment Setup 🔧

1. **Install PlatformIO**:
   - Download and install PlatformIO as a Visual Studio Code extension or use the command line (recommended).

2. **Configure the ESP32 Environment**:
   - Make sure to have installed:
     - **ESP-IDF**: The specific framework for ESP32, useful for advanced functionalities and low-level configurations.
     - **Arduino Framework**: Required for the application code.
     - **A ESP32 Server Library** for Arduino (like [esp32_https_server](https://github.com/fhessel/esp32_https_server), deprecated but good) or change the setup with ESP-IDF Framework.

3. **Generate Certificates and Secrets**:
   - Use the configuration scripts `generate_certs.bash` and `generate_secret.bash` included in the project folder:
     ```bash
     ./generate_certs.bash
     ./generate_secret.bash
     ```
   - These scripts will automate the generation of HTTPS certificates and the secrets necessary for secure authentication.

### 🔨 Build, Compile, Upload, and Flash to ESP32 🔨

A. **Using PlatformIO with the Arduino Framework**

   If you prefer to use the Arduino framework within PlatformIO, follow these steps to compile and upload:

   1. **Compile and Upload**:
      - Once the environment is set up, use PlatformIO to compile and upload the code to the ESP32 modules:
      ```bash
      pio run -t upload
      pio run -t clean
      ```
      
   2. **Monitor Serial Output**:
      - Verify the configuration by monitoring the serial output for debug messages and server status (specify the port if needed):
      ```bash
      pio device monitor
      ```

B. **Manual Setup Using ESP-IDF**

   Alternatively, you can manually compile and flash the project using ESP-IDF and `idf.py`. This method is ideal if you need direct control over the ESP-IDF framework.

   1. **Set Up the ESP-IDF Environment**:
      - Initialize the ESP-IDF environment by running:
      ```bash
      . $HOME/esp/esp-idf/export.sh
      ```
      - Replace `$HOME/esp/esp-idf` with the path where ESP-IDF is installed.

   2. **Configure the ESP32 Target**:
      - Set the target device to ESP32:
      ```bash
      idf.py set-target esp32
      ```

   3. **Build the Project**:
      - Compile the project using:
      ```bash
      idf.py build
      ```
      - This command generates the binary files needed to flash the ESP32.

   4. **Flash the Firmware onto ESP32**:
      - Connect the ESP32 via USB and use the following command to flash the firmware:
      ```bash
      idf.py -p /dev/ttyUSB0 flash
      ```
      - Replace `/dev/ttyUSB0` with the correct serial port for your system (for example, it could be `COM3` on Windows or `/dev/ttyUSB1` on Linux).

   5. **Monitor the Serial Output**:
      - After flashing, you can monitor the serial output to check for debug messages and confirm successful operation:
      ```bash
      idf.py -p /dev/ttyUSB0 monitor
      ```
      - Use `Ctrl + ]` to exit the monitor.

### 🧪 Test the Connection 🧪
- Ensure the devices are properly connected and communicate via the chosen protocol (HTTP/HTTPS).
- For a successful client-server connection, make sure to configure the following parameters on the client side:
  - **Server Address**: Specify the server’s IP address or hostname that the client will connect to.
  - **Connection Preference (HTTP/HTTPS)**: Choose the desired protocol. If using HTTPS, ensure:
    - The client is configured to send its certificates for mutual authentication, if required by the server.
    - Client certificates and keys are correctly set up, especially if the server requires client authentication.
  - **SSID and Password**: Set the correct Wi-Fi SSID and password for network access to allow the ESP32 modules to connect to the desired network.
- With these parameters correctly configured, initiate the connection and verify that the client can establish communication with the server via the specified protocol.


## 🙏 Acknowledgements

### ESP-IDF 📘

ESP-IDF is the official Espressif framework for ESP32 development. It supports advanced features such as network management, encryption, and secure communication protocols, making it ideal for IoT projects requiring robustness and security.

[More information here](https://github.com/espressif/esp-idf)

### Arduino ⚙️

Arduino provides a user-friendly and versatile framework for embedded device programming. It is used in this project for basic configurations and client-server interface management.

[More information here](https://www.arduino.cc/)

### C++ 💻

Some components of the project are implemented in C++ for its efficiency and advanced memory management, which is crucial for resource-limited devices like the ESP32.

### Networking & Sniffing 🌐🔍

This project explores not only secure client-server communication but also demonstrates the security vulnerabilities that can arise in unsecured HTTP sessions. In particular, it highlights how an HTTP connection is vulnerable to replay attacks in the presence of a Man-in-the-Middle (MitM) attack, while an HTTPS connection, secured with certificates, mitigates such risks.

To demonstrate these concepts, several network sniffing and interception techniques were employed:

- **ESP32 Marauder Mini**: Using an ESP32 Marauder with [Marauder firmware](https://github.com/justcallmekoko/ESP32Marauder), network interception was achieved through sniffing methods like `sniffesp` and `sniffraw`. By cloning an access point (AP), it was possible to observe and freely read the HTTP connections without encryption and save them in .pcap files in the SD memory integrated, more information about it [here](https://github.com/justcallmekoko/ESP32Marauder/wiki).

- **Proxy Interception**: In another approach, an intermediary device was used as a proxy to capture HTTP traffic, enabling the analysis and testing of connection security under different configurations, just use another device to intercept (like a proxy http server) the network traffic and save it in .pcap file. Do not save in the memory of ESP32 but in a memory support or send them over the network to save elsewhere.

- **ARP Poisoning**: To capture the traiffic, ARP poisoning was used to route traffic through a third device (our machine). This method allowed the captured traffic with [Wireshark](https://www.wireshark.org/) to be saved in `.pcap` format, which was then replayed with tools such as `tcpreplay` or other utilities within `Npcap` to analyze the security of the session.

These techniques provide insights into potential vulnerabilities of unencrypted communications in IoT environments and underscore the importance of using HTTPS for secure data transmission.


## 📜 License

This project is licensed under the GNU General Public License v3.0.

[GNU General Public License v3.0](https://www.gnu.org/licenses/gpl-3.0.en.html)

![Static Badge](https://img.shields.io/badge/UniSA-IoTSecurity-red?style=plastic)


## 🖐 Authors

**Contributors:**
- [Fulvio Serao](https://github.com/Fulvioserao99)

**Project Manager:**
- [Antonio Garofalo](https://github.com/Endless077)


## 🔔 Support

For support, email [antonio.garofalo125@gmail.com](mailto:antonio.garofalo125@gmail.com) or contact the project contributors.


### 📝 Documentation

See the full project documentation **[here](https://github.com/Endless077/IoT_Security_Networking/blob/main/docs.pdf)**.
