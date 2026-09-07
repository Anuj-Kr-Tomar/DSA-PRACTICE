#include <iostream>
#include <string>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

// CRC-8: x^8 + x^2 + x + 1
const string GENERATOR = "100000111";

// Convert a string into 8-bit ASCII binary
string stringToBinary(string text)
{
    string binary = "";

    for (char ch : text)
    {
        for (int i = 7; i >= 0; i--)
        {
            if (ch & (1 << i))
                binary += '1';
            else
                binary += '0';
        }
    }

    return binary;
}

// Calculate CRC using modulo-2 XOR division
string calculateCRC(string data)
{
    int n = GENERATOR.length();

    // Append n-1 zeros
    string temp = data + string(n - 1, '0');

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

    // CRC is last 8 bits
    return temp.substr(temp.length() - 8);
}

int main()
{
    int clientSocket;
    struct sockaddr_in serverAddress;

    cout << "========================================" << endl;
    cout << "          CRC SENDER / CLIENT           " << endl;
    cout << "========================================" << endl;

    // Take first name from user
    string name;

    cout << "Enter your first name: ";
    cin >> name;

    // Convert name to binary
    string data = stringToBinary(name);

    cout << "\nOriginal Data: " << name << endl;
    cout << "Binary Data: " << data << endl;
    cout << "Generator: " << GENERATOR << endl;

    // Calculate CRC
    string crc = calculateCRC(data);

    cout << "CRC: " << crc << endl;

    // Create codeword
    string codeword = data + crc;

    cout << "Codeword: " << codeword << endl;

    // Create TCP socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket < 0)
    {
        cout << "Socket creation failed!" << endl;
        return 1;
    }

    // Server address
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.7");

    // Connect to receiver
    if (connect(clientSocket,
                (struct sockaddr *)&serverAddress,
                sizeof(serverAddress)) < 0)
    {
        cout << "\nConnection failed!" << endl;
        cout << "Please start the receiver first." << endl;

        close(clientSocket);
        return 1;
    }

    cout << "\nConnected to receiver." << endl;

    // Send codeword
    send(clientSocket,
         codeword.c_str(),
         codeword.length(),
         0);

    cout << "Codeword sent successfully!" << endl;

    // Receive result from receiver
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    int bytesReceived = recv(clientSocket,
                             buffer,
                             sizeof(buffer) - 1,
                             0);

    if (bytesReceived > 0)
    {
        buffer[bytesReceived] = '\0';

        cout << "\nMessage from Receiver: "
             << buffer << endl;
    }

    close(clientSocket);

    cout << "\nSender finished." << endl;

    return 0;
}
