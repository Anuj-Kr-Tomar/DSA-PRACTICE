#include <iostream>
// Used for input and output.
// Provides cout and cin.

#include <string>
// Used for the C++ string data type.

#include <cstring>
// Provides functions for handling character arrays,
// especially memset() used for clearing the receiving buffer.

#include <arpa/inet.h>
// Provides networking functions such as:
// inet_addr() and htons().

#include <sys/socket.h>
// Provides socket functions such as:
// socket(), connect(), send(), recv().

#include <unistd.h>
// Provides close(), which is used to close the socket.

using namespace std;
// Allows us to use string, cout, cin etc.
// without writing std:: before them.


// ==========================================================
//                    CRC CONFIGURATION
// ==========================================================

// CRC-8 generator polynomial:
// x^8 + x^2 + x + 1
//
// Binary representation:
// 100000111
//
// Degree of polynomial = 8
// Therefore, CRC remainder = 8 bits.
const string GENERATOR = "100000111";


// ==========================================================
//                  CONVERT STRING TO BINARY
// ==========================================================

// This function converts the user's name
// into its 8-bit ASCII binary representation.
//
// Example:
// A → ASCII 65 → 01000001
string stringToBinary(string text)
{
    // Create an empty string to store
    // the final binary representation.
    string binary = "";


    // Loop through every character of the input string.
    //
    // For example, if text = "ANUJ",
    // this loop processes:
    // A → N → U → J
    for (char ch : text)
    {

        // Each character is represented using 8 bits.
        //
        // We check bits from position 7 down to position 0.
        for (int i = 7; i >= 0; i--)
        {

            // (1 << i) creates a value with
            // the i-th bit set to 1.
            //
            // The '&' operator checks whether
            // the corresponding bit of 'ch' is 1.
            if (ch & (1 << i))
            {
                // If the bit is 1,
                // add '1' to the binary string.
                binary += '1';
            }

            else
            {
                // If the bit is 0,
                // add '0' to the binary string.
                binary += '0';
            }
        }
    }


    // Return the complete binary representation
    // of the input text.
    return binary;
}


// ==========================================================
//                    CALCULATE CRC
// ==========================================================

// This function calculates the CRC value
// for the original binary data.
//
// It uses modulo-2 division,
// where XOR is used instead of normal subtraction.
string calculateCRC(string data)
{
    // Store the length of the generator.
    //
    // GENERATOR = 100000111
    // Length = 9
    int n = GENERATOR.length();


    // Append n-1 zeros to the original data.
    //
    // Since n = 9:
    // n - 1 = 8
    //
    // Therefore, we append 8 zeros.
    //
    // Example:
    // Data = 10110101
    //
    // temp =
    // 1011010100000000
    string temp = data + string(n - 1, '0');


    // Perform modulo-2 division.
    //
    // We move from left to right through the data.
    for (int i = 0;
         i <= (int)temp.length() - n;
         i++)
    {

        // We perform XOR with the generator
        // only if the current bit is 1.
        if (temp[i] == '1')
        {

            // Process all 9 bits of the generator.
            for (int j = 0; j < n; j++)
            {

                // XOR operation:
                //
                // Same bits:
                // 0 XOR 0 = 0
                // 1 XOR 1 = 0
                //
                // Different bits:
                // 0 XOR 1 = 1
                // 1 XOR 0 = 1
                //
                // So if both bits are same,
                // store 0.
                if (temp[i + j] == GENERATOR[j])
                    temp[i + j] = '0';

                // If the bits are different,
                // store 1.
                else
                    temp[i + j] = '1';
            }
        }
    }


    // After modulo-2 division,
    // the last 8 bits contain the CRC remainder.
    //
    // We return these 8 bits.
    return temp.substr(temp.length() - 8);
}


// ==========================================================
//                         MAIN
// ==========================================================

int main()
{
    // This variable stores the socket descriptor
    // used by the client for communication.
    int clientSocket;


    // Structure used to store the server's
    // IP address and port information.
    struct sockaddr_in serverAddress;


    // ======================================================
    //                    DISPLAY TITLE
    // ======================================================

    cout << "========================================" << endl;

    cout << "          CRC SENDER / CLIENT           " << endl;

    cout << "========================================" << endl;


    // ======================================================
    //                    TAKE USER INPUT
    // ======================================================

    // Variable to store the user's name.
    string name;


    // Ask the user to enter their first name.
    cout << "Enter your first name: ";


    // Read the name from the keyboard.
    //
    // cin >> name reads one word.
    cin >> name;


    // ======================================================
    //                CONVERT NAME TO BINARY
    // ======================================================

    // Call stringToBinary() to convert
    // the name into 8-bit ASCII binary.
    string data = stringToBinary(name);


    // Display the original name.
    cout << "\nOriginal Data: "
         << name << endl;


    // Display the binary representation.
    cout << "Binary Data: "
         << data << endl;


    // Display the generator polynomial
    // used for CRC.
    cout << "Generator: "
         << GENERATOR << endl;


    // ======================================================
    //                    CALCULATE CRC
    // ======================================================

    // Calculate the CRC remainder
    // for the binary data.
    string crc = calculateCRC(data);


    // Display the calculated CRC.
    cout << "CRC: "
         << crc << endl;


    // ======================================================
    //                  CREATE CODEWORD
    // ======================================================

    // Append the CRC to the original binary data.
    //
    // Codeword = Data + CRC
    //
    // Example:
    //
    // Data = 10110101
    // CRC  = 11001010
    //
    // Codeword =
    // 1011010111001010
    string codeword = data + crc;


    // Display the final codeword.
    cout << "Codeword: "
         << codeword << endl;


    // ======================================================
    //                  CREATE TCP SOCKET
    // ======================================================

    // Create a TCP socket.
    //
    // AF_INET:
    //     IPv4
    //
    // SOCK_STREAM:
    //     TCP
    //
    // 0:
    //     Default protocol for TCP.
    clientSocket = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );


    // Check if socket creation failed.
    //
    // A negative value means failure.
    if (clientSocket < 0)
    {

        cout << "Socket creation failed!"
             << endl;

        // Terminate the program.
        return 1;
    }


    // ======================================================
    //                  SERVER ADDRESS
    // ======================================================

    // Specify that the server uses IPv4.
    serverAddress.sin_family = AF_INET;


    // Specify the server's port number.
    //
    // Server is listening on port 8080.
    //
    // htons() converts the port number
    // into network byte order.
    serverAddress.sin_port = htons(8080);


    // Specify the IP address of the server.
    //
    // 127.0.0.7 is a loopback address,
    // so the communication is happening
    // locally on the same computer.
    serverAddress.sin_addr.s_addr =
        inet_addr("127.0.0.7");


    // ======================================================
    //                   CONNECT TO SERVER
    // ======================================================

    // connect() establishes a TCP connection
    // between the client and the server.
    //
    // clientSocket:
    //     Our client socket.
    //
    // serverAddress:
    //     Address of the server.
    if (connect(
            clientSocket,
            (struct sockaddr *)&serverAddress,
            sizeof(serverAddress)
        ) < 0)
    {

        // If connection fails,
        // display an error message.
        cout << "\nConnection failed!"
             << endl;


        // Tell the user that the receiver
        // should be started first.
        cout << "Please start the receiver first."
             << endl;


        // Close the socket because
        // connection was unsuccessful.
        close(clientSocket);


        // Terminate the program.
        return 1;
    }


    // If we reach here,
    // TCP connection was successfully established.
    cout << "\nConnected to receiver."
         << endl;


    // ======================================================
    //                    SEND CODEWORD
    // ======================================================

    // Send the codeword to the receiver.
    //
    // codeword.c_str():
    //     Converts C++ string into C-style character array.
    //
    // codeword.length():
    //     Number of characters/bits to send.
    //
    // 0:
    //     No special flags.
    send(
        clientSocket,
        codeword.c_str(),
        codeword.length(),
        0
    );


    // Display confirmation.
    cout << "Codeword sent successfully!"
         << endl;


    // ======================================================
    //                  RECEIVE RESULT
    // ======================================================

    // Create a buffer to store
    // the response from the receiver.
    char buffer[1024];


    // Initialize the buffer with zeros.
    //
    // This removes any garbage values
    // that may be present in memory.
    memset(
        buffer,
        0,
        sizeof(buffer)
    );


    // Receive the result from the receiver.
    //
    // The receiver will send either:
    //
    // "No Error Detected."
    //
    // OR
    //
    // "Transmission Error Detected!"
    int bytesReceived = recv(
        clientSocket,
        buffer,
        sizeof(buffer) - 1,
        0
    );


    // Check whether we actually received data.
    if (bytesReceived > 0)
    {

        // Add null character at the end
        // to make buffer a valid C-style string.
        buffer[bytesReceived] = '\0';


        // Display the result received
        // from the receiver.
        cout << "\nMessage from Receiver: "
             << buffer << endl;
    }


    // ======================================================
    //                    CLOSE SOCKET
    // ======================================================

    // Close the TCP connection.
    close(clientSocket);


    // Display completion message.
    cout << "\nSender finished."
         << endl;


    // Return 0 means the program
    // completed successfully.
    return 0;
}
