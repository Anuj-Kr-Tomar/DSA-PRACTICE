#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

using namespace std;

// CRC-8: x^8 + x^2 + x + 1
const string GENERATOR = "100000111";

// Check CRC using modulo-2 XOR division
string calculateRemainder(string codeword)
{
    int n = GENERATOR.length();

    string temp = codeword;

    for (int i = 0; i <= (int)temp.length() - n; i++)
    {
        if (temp[i] == '1')
        {
            for (int j = 0; j < n; j++)
            {
                if (temp[i + j] == GENERATOR[j])
                    temp[i + j] = '0';
                else
                    temp[i + j] = '1';
            }
        }
    }

    // Last 8 bits are remainder
    return temp.substr(temp.length() - 8);
}

int main()
{
    int serverSocket;
    int clientSocket;

    struct sockaddr_in serverAddress;
    struct sockaddr_in clientAddress;

    socklen_t clientLength = sizeof(clientAddress);

    cout << "========================================" << endl;
    cout << "          CRC RECEIVER / SERVER         " << endl;
    cout << "========================================" << endl;

    // Create TCP socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket < 0)
    {
        cout << "Socket creation failed!" << endl;
        return 1;
    }

    // Allow reuse of port
    int option = 1;

    setsockopt(serverSocket,
               SOL_SOCKET,
               SO_REUSEADDR,
               &option,
               sizeof(option));

    // Server address
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8080);

    // Bind socket
    if (bind(serverSocket,
             (struct sockaddr *)&serverAddress,
             sizeof(serverAddress)) < 0)
    {
        cout << "Bind failed!" << endl;
        close(serverSocket);
        return 1;
    }

    // Listen
    if (listen(serverSocket, 5) < 0)
    {
        cout << "Listen failed!" << endl;
        close(serverSocket);
        return 1;
    }

    cout << "Waiting for sender..." << endl;

    // Accept connection
    clientSocket = accept(serverSocket,
                           (struct sockaddr *)&clientAddress,
                           &clientLength);

    if (clientSocket < 0)
    {
        cout << "Accept failed!" << endl;
        close(serverSocket);
        return 1;
    }

    cout << "Sender connected!" << endl;

    // Receive codeword
    char buffer[10000];
    memset(buffer, 0, sizeof(buffer));

    int bytesReceived = recv(clientSocket,
                             buffer,
                             sizeof(buffer) - 1,
                             0);

    if (bytesReceived <= 0)
    {
        cout << "No data received!" << endl;

        close(clientSocket);
        close(serverSocket);

        return 1;
    }

    buffer[bytesReceived] = '\0';

    string codeword = buffer;

    cout << "Generator: "
         << GENERATOR << endl;

    // Receiver chooses test
    int choice;

    cout << "\nChoose Test:" << endl;
    cout << "1. No Error" << endl;
    cout << "2. Introduce Single-Bit Error" << endl;
    cout << "Enter choice: ";

    cin >> choice;
    if (choice ==1){
        cout << "\nReceived Codeword: "
         << codeword << endl;
    }
    // Single-bit error
    else if (choice == 2)
    {
        srand(time(0));

        // Random position
        int randomPosition =
            rand() % codeword.length();

        // Change bit
        if (codeword[randomPosition] == '0')
            codeword[randomPosition] = '1';
        else
            codeword[randomPosition] = '0';

        cout << "Received Codeword:: "
             << codeword << endl;
    }
    else
    {
        cout << "Invalid choice!" << endl;

        close(clientSocket);
        close(serverSocket);

        return 1;
    }

    // Calculate remainder
    cout << "\nChecking CRC..." << endl;

    string remainder = calculateRemainder(codeword);

    cout << "Remainder: "
         << remainder << endl;

    string message;

    // Check remainder
    if (remainder == "00000000")
    {
        cout << "No Error Detected." << endl;

        message = "No Error Detected.";
    }
    else
    {
        cout << "Transmission Error Detected!" << endl;

        message = "Transmission Error Detected!";
    }

    // Send result back to sender
    send(clientSocket,
         message.c_str(),
         message.length(),
         0);

    cout << "Result sent to sender." << endl;

    close(clientSocket);
    close(serverSocket);

    return 0;
}
