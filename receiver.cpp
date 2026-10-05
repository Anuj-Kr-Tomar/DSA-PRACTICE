#include <iostream>
// Used for input and output.
// Provides cout and cin.

#include <string>
// Used for the C++ string data type.

#include <cstring>
// Provides functions for handling C-style strings,
// especially memset() used for clearing the buffer.

#include <cstdlib>
// Provides functions like rand() and srand()
// which we use to generate a random position for bit flipping.

#include <ctime>
// Provides time(0), which is used to create
// a different random seed each time the program runs.

#include <sys/socket.h>
// Provides socket-related functions such as:
// socket(), bind(), listen(), accept(), send(), recv().

#include <netinet/in.h>
// Provides structures and constants used for IPv4 networking,
// such as sockaddr_in and INADDR_ANY.

#include <unistd.h>
// Provides close(), which is used to close sockets.

using namespace std;
// Allows us to directly use string, cout, cin etc.
// instead of writing std::string, std::cout, std::cin.

//              SENDER / CLIENT
//                   |
//                   |
//           Enter name "ANUJ"
//                   |
//                   ↓
//         Convert to ASCII binary
//                   |
//                   ↓
//        Calculate CRC-8 remainder
//                   |
//                   ↓
//        Data + CRC = Codeword
//                   |
//                   ↓
//         TCP connection to server
//                   |
//                   ↓
//           Send Codeword
//                   |
//                   ↓
//         --------------------
//         |                  |
//         |     NETWORK      |
//         |                  |
//         --------------------
//                   |
//                   ↓
//              RECEIVER
//                   |
//                   ↓
//         Receive Codeword
//                   |
//                   ↓
//        Choose test:
//        1. No Error
//        2. Single-bit Error
//                   |
//                   ↓
//        Calculate CRC remainder
//                   |
//              /----------\
//             /            \
//     00000000              Non-zero
//        |                     |
//        ↓                     ↓
//    No Error              Error Detected
//        |                     |
//        \----------┬----------/
//                   ↓
//         Send result to sender

// ==========================================================
//                  CRC CONFIGURATION
// ==========================================================

// CRC-8 generator polynomial:
// x^8 + x^2 + x + 1
//
// Its binary representation is:
// 100000111
//
// Since the degree of the polynomial is 8,
// the CRC remainder will be 8 bits.
const string GENERATOR = "100000111";


// ==========================================================
//             CRC REMAINDER CALCULATION
// ==========================================================

// This function takes the complete received codeword
// and calculates its CRC remainder.
//
// At the receiver, we do NOT append zeros again.
// We directly divide the received codeword
// by the generator using modulo-2 division.
string calculateRemainder(string codeword)
{
    // Store the length of the generator.
    // GENERATOR = "100000111"
    // Length = 9
    int n = GENERATOR.length();


    // Make a temporary copy of the codeword.
    // We perform XOR operations on 'temp'
    // instead of modifying the original codeword.
    string temp = codeword;


    // This loop performs modulo-2 division.
    // We move from left to right through the data.
    // n = 9 because our generator contains 9 bits.
    for (int i = 0; i <= (int)temp.length() - n; i++)
    {

        // We perform XOR with the generator
        // only when the current bit is 1.
        // If the current bit is 0,
        // there is no need to perform XOR.
        if (temp[i] == '1')
        {

            // Compare/XOR all 9 bits of the generator
            // with the corresponding 9 bits of temp.
            for (int j = 0; j < n; j++)
            {

                // XOR logic:
                // Same bits:
                // 0 XOR 0 = 0
                // 1 XOR 1 = 0
                // Different bits:
                // 0 XOR 1 = 1
                // 1 XOR 0 = 1
                // If both bits are same, store 0.
                if (temp[i + j] == GENERATOR[j])
                    temp[i + j] = '0';

                // If bits are different, store 1.
                else
                    temp[i + j] = '1';
            }
        }
    }


    // After modulo-2 division,
    // the last 8 bits contain the CRC remainder.
    // Why 8?
    // Because this is CRC-8 and the generator
    // has degree 8.
    return temp.substr(temp.length() - 8);
}


// ==========================================================
//                         MAIN
// ==========================================================

int main()
{

    // serverSocket is the main socket used by the server
    // to listen for incoming client connections.
    int serverSocket;


    // clientSocket is the socket created after a client
    // connects to the server.
    //
    // This socket is used for actual communication
    // between sender and receiver.
    int clientSocket;


    // Structure used to store the server's
    // IPv4 address and port information.
    struct sockaddr_in serverAddress;


    // Structure used to store the client's
    // IPv4 address information.
    struct sockaddr_in clientAddress;


    // Store the size of clientAddress.
    //
    // accept() needs this information.
    socklen_t clientLength = sizeof(clientAddress);


    // ------------------------------------------------------
    //                 DISPLAY PROGRAM NAME
    // ------------------------------------------------------

    cout << "========================================" << endl;

    cout << "          CRC RECEIVER / SERVER         " << endl;

    cout << "========================================" << endl;


    // ======================================================
    //                  CREATE TCP SOCKET
    // ======================================================

    // socket() creates a communication endpoint.
    //
    // AF_INET:
    //     Use IPv4.
    //
    // SOCK_STREAM:
    //     Use TCP.
    //
    // 0:
    //     Use the default protocol for TCP.
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);


    // Check whether socket creation was successful.
    //
    // A negative value means socket creation failed.
    if (serverSocket < 0)
    {
        cout << "Socket creation failed!" << endl;

        // Stop the program because
        // communication cannot happen without a socket.
        return 1;
    }


    // ======================================================
    //                ALLOW PORT REUSE
    // ======================================================

    // option = 1 means enable the socket option.
    int option = 1;


    // setsockopt() is used to configure socket options.
    //
    // SO_REUSEADDR allows the server to reuse
    // the same port more easily after restarting.
    setsockopt(
        serverSocket,       // Socket we want to configure
        SOL_SOCKET,         // Socket-level option
        SO_REUSEADDR,       // Allow address/port reuse
        &option,            // Value of the option
        sizeof(option)      // Size of the option
    );


    // ======================================================
    //                SET SERVER ADDRESS
    // ======================================================

    // Tell the socket that we are using IPv4.
    serverAddress.sin_family = AF_INET;


    // INADDR_ANY means the server can accept connections
    // from any network interface available on this machine.
    serverAddress.sin_addr.s_addr = INADDR_ANY;


    // Set the server port to 8080.
    //
    // htons() converts the port number
    // from host byte order to network byte order.
    serverAddress.sin_port = htons(8080);


    // ======================================================
    //                         BIND
    // ======================================================

    // bind() associates the socket with
    // the specified IP address and port.
    //
    // In simple words:
    // bind() gives the server its address and port.
    if (bind(
            serverSocket,
            (struct sockaddr *)&serverAddress,
            sizeof(serverAddress)
        ) < 0)
    {

        cout << "Bind failed!" << endl;


        // Close the socket because the server
        // cannot continue without successful binding.
        close(serverSocket);

        return 1;
    }


    // ======================================================
    //                        LISTEN
    // ======================================================

    // listen() puts the server socket into
    // listening mode.
    //
    // 5 is the backlog value.
    // It specifies how many connection requests
    // can wait in the queue.
    if (listen(serverSocket, 5) < 0)
    {

        cout << "Listen failed!" << endl;


        // Close the server socket.
        close(serverSocket);

        return 1;
    }


    // Server is now ready and waiting for
    // a sender/client to connect.
    cout << "Waiting for sender..." << endl;


    // ======================================================
    //                        ACCEPT
    // ======================================================

    // accept() waits for a client to connect.
    //
    // Once a client connects:
    // accept() returns a NEW socket called clientSocket.
    //
    // serverSocket continues to be the listening socket.
    // clientSocket is used for actual communication.
    clientSocket = accept(
        serverSocket,
        (struct sockaddr *)&clientAddress,
        &clientLength
    );


    // Check whether accepting the connection failed.
    if (clientSocket < 0)
    {
        cout << "Accept failed!" << endl;


        // Close the listening socket.
        close(serverSocket);

        return 1;
    }


    // This means the sender/client successfully connected.
    cout << "Sender connected!" << endl;


    // ======================================================
    //                    RECEIVE CODEWORD
    // ======================================================

    // Create a character array to temporarily
    // store the data received from the sender.
    char buffer[10000];


    // Initialize the entire buffer with zeros.
    //
    // This prevents garbage values from remaining
    // in the buffer.
    memset(buffer, 0, sizeof(buffer));


    // recv() receives data from the sender.
    //
    // clientSocket:
    //     Socket used for communication.
    //
    // buffer:
    //     Where received data will be stored.
    //
    // sizeof(buffer) - 1:
    //     Maximum amount of data to receive.
    //
    // 0:
    //     No special flags.
    int bytesReceived = recv(
        clientSocket,
        buffer,
        sizeof(buffer) - 1,
        0
    );


    // If bytesReceived is 0 or negative,
    // no valid data was received.
    if (bytesReceived <= 0)
    {

        cout << "No data received!" << endl;


        // Close both sockets.
        close(clientSocket);
        close(serverSocket);

        return 1;
    }


    // Add '\0' at the end of received data.
    //
    // This makes the character array a valid
    // C-style string.
    buffer[bytesReceived] = '\0';


    // Convert the received character array
    // into a C++ string.
    //
    // Now 'codeword' contains the data
    // sent by the sender.
    string codeword = buffer;


    // Display the generator being used for CRC checking.
    cout << "Generator: "
         << GENERATOR << endl;


    // ======================================================
    //                  CHOOSE TEST CASE
    // ======================================================

    // Variable used to store the user's choice.
    int choice;


    cout << "\nChoose Test:" << endl;


    // Test case 1:
    // Do not change the received codeword.
    cout << "1. No Error" << endl;


    // Test case 2:
    // Intentionally change one bit
    // to simulate a transmission error.
    cout << "2. Introduce Single-Bit Error" << endl;


    // Ask the user to select the test.
    cout << "Enter choice: ";

    cin >> choice;


    // ======================================================
    //                     TEST CASE 1
    // ======================================================

    // If user selects 1,
    // we do not modify the received codeword.
    if (choice == 1)
    {

        // Display the original received codeword.
        cout << "\nReceived Codeword: "
             << codeword << endl;
    }


    // ======================================================
    //                     TEST CASE 2
    // ======================================================

    // If user selects 2,
    // we intentionally introduce one-bit error.
    else if (choice == 2)
    {

        // Initialize the random number generator
        // using the current time.
        //
        // This helps us get a different random position
        // each time the program runs.
        srand(time(0));


        // Generate a random position between:
        //
        // 0 and codeword.length() - 1
        //
        // This position will be used for flipping one bit.
        int randomPosition =
            rand() % codeword.length();


        // ==================================================
        //                 FLIP ONE BIT
        // ==================================================

        // If the selected bit is 0,
        // change it to 1.
        if (codeword[randomPosition] == '0')
            codeword[randomPosition] = '1';


        // Otherwise, if the selected bit is 1,
        // change it to 0.
        else
            codeword[randomPosition] = '0';


        // Display the codeword after
        // introducing the artificial error.
        cout << "Received Codeword:: "
             << codeword << endl;
    }


    // ======================================================
    //                   INVALID CHOICE
    // ======================================================

    // If the user enters something other than 1 or 2.
    else
    {

        cout << "Invalid choice!" << endl;


        // Close both sockets.
        close(clientSocket);
        close(serverSocket);

        return 1;
    }


    // ======================================================
    //                   CRC VERIFICATION
    // ======================================================

    // Tell the user that CRC checking is starting.
    cout << "\nChecking CRC..." << endl;


    // Perform modulo-2 XOR division
    // on the received codeword.
    //
    // The result is the CRC remainder.
    string remainder =
        calculateRemainder(codeword);


    // Display the calculated remainder.
    cout << "Remainder: "
         << remainder << endl;


    // This string will contain the final result
    // that we send back to the sender.
    string message;


    // ======================================================
    //                  CHECK CRC RESULT
    // ======================================================

    // If the remainder contains all zeros,
    // no error is detected.
    if (remainder == "00000000")
    {

        cout << "No Error Detected." << endl;


        // Store the result in message
        // so that we can send it back to the sender.
        message = "No Error Detected.";
    }


    // If remainder is not zero,
    // an error has been detected.
    else
    {

        cout << "Transmission Error Detected!" << endl;


        // Store the error message
        // so that it can be sent back to the sender.
        message = "Transmission Error Detected!";
    }


    // ======================================================
    //                 SEND RESULT TO SENDER
    // ======================================================

    // Send the CRC result back to the sender.
    //
    // message.c_str():
    //     Converts C++ string into C-style character array.
    //
    // message.length():
    //     Specifies the number of characters to send.
    //
    // 0:
    //     No special flags.
    send(
        clientSocket,
        message.c_str(),
        message.length(),
        0
    );


    // Display confirmation that
    // the result was successfully sent.
    cout << "Result sent to sender." << endl;


    // ======================================================
    //                    CLOSE SOCKETS
    // ======================================================

    // Close the connection with the sender.
    close(clientSocket);


    // Close the server/listening socket.
    close(serverSocket);


    // Return 0 means the program completed successfully.
    return 0;
}
