#include <iostream>       // Provides cout and endl for output.
#include <cstdlib>        // Provides general utility functions.
#include <cstring>        // Provides strcpy(), strcmp(), strlen(), etc.
#include <arpa/inet.h>    // Provides IPv4 socket/network functions.
#include <unistd.h>       // Provides close().

using namespace std;      // Lets us use cout instead of std::cout.

#define PORT 9000         // Receiver UDP port.
#define TOTAL_FRAMES 8    // Total number of frames.

int main()                  // Program starts here.
{
    int sock;               // Stores the socket descriptor.
    int expected = 0;       // GBN: next frame expected by receiver.
    int received[TOTAL_FRAMES] = {0}; // SR: 0 = not received, 1 = received/buffered.
    char buffer[50];        // Stores received messages.
    char protocol[10];      // Stores "GBN" or "SR".
    struct sockaddr_in receiver{}, sender{}; // Stores IPv4 address information.
    socklen_t length = sizeof(sender); // Stores sender address size.

    // CREATE SOCKET
    sock = socket(AF_INET, SOCK_DGRAM, 0); // Creates an IPv4 UDP socket.

    if (sock < 0) // Checks whether socket creation failed.
    {
        perror("Socket creation failed"); // Prints the system error.
        return 1; // Exits with an error.
    }

    // RECEIVER ADDRESS
    receiver.sin_family = AF_INET; // Uses IPv4.
    receiver.sin_port = htons(PORT); // Converts port 9000 to network byte order.
    receiver.sin_addr.s_addr = inet_addr("127.0.0.7"); // Sets receiver IP.

    if (bind(sock, (struct sockaddr *)&receiver, sizeof(receiver)) < 0) // Binds socket to IP and port.
    {
        perror("Receiver bind failed"); // Prints bind error.
        close(sock); // Closes socket.
        return 1; // Exits with an error.
    }

    cout << "=====================================\n"; // Prints separator.
    cout << " SLIDING WINDOW RECEIVER\n"; // Prints receiver heading.
    cout << "=====================================\n"; // Prints separator.
    cout << "Roll Number : 7\n"; // Displays roll number.
    cout << "Receiver IP : 127.0.0.7\n"; // Displays receiver IP.
    cout << "Receiver Port: 9000\n"; // Displays receiver port.
    cout << "\nWaiting for sender...\n"; // Indicates waiting state.

    // RECEIVE PROTOCOL
    int n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&sender, &length); // Receives GBN/SR.

    if (n <= 0) // Checks whether reception failed.
    {
        cout << "Failed to receive protocol.\n"; // Displays failure.
        close(sock); // Closes socket.
        return 1; // Exits.
    }

    buffer[n] = '\0'; // Terminates the received string.
    strcpy(protocol, buffer); // Copies protocol into protocol variable.
    cout << "\nProtocol: " << protocol << endl; // Displays selected protocol.

    // GO BACK N
    if (strcmp(protocol, "GBN") == 0) // Checks whether GBN was selected.
    {
        cout << "\n--- GO BACK N RECEIVER ---\n"; // Displays GBN heading.

        while (expected < TOTAL_FRAMES) // Runs until all frames are received in order.
        {
            n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&sender, &length); // Receives DATA frame.

            if (n <= 0) // Checks receive failure.
                continue; // Waits for another frame.

            buffer[n] = '\0'; // Terminates received string.
            int frame; // Stores frame number.
            sscanf(buffer, "DATA:%d", &frame); // Extracts frame number from DATA:n.
            cout << "\nReceived DATA " << frame << endl; // Displays received frame.

            if (frame == expected) // Accepts only the expected frame.
            {
                cout << "Frame " << frame << " accepted\n"; // Reports acceptance.

                int ackNumber = frame + 1; // ACK n+1 acknowledges DATA n.
                sprintf(buffer, "ACK:%d", ackNumber); // Creates ACK message.
                sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)&sender, length); // Sends ACK.
                cout << "Sent ACK " << ackNumber << endl; // Displays ACK.
                expected++; // Moves expected frame forward.
            }
            else // Executes for an out-of-order frame.
            {
                cout << "Frame " << frame << " discarded\n"; // GBN discards out-of-order frames.
                sprintf(buffer, "ACK:%d", expected); // Sends ACK for the next expected frame.
                sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)&sender, length); // Sends duplicate/cumulative ACK.
                cout << "Sent ACK " << expected << endl; // Displays ACK.
            }
        }
    }

    // SELECTIVE REPEAT
    else if (strcmp(protocol, "SR") == 0) // Checks whether SR was selected.
    {
        cout << "\n--- SELECTIVE REPEAT RECEIVER ---\n"; // Displays SR heading.

        while (expected < TOTAL_FRAMES) // Runs until all frames can be delivered in order.
        {
            n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&sender, &length); // Receives DATA.

            if (n <= 0) // Checks receive failure.
                continue; // Waits for another packet.

            buffer[n] = '\0'; // Terminates received string.
            int frame; // Stores frame number.
            sscanf(buffer, "DATA:%d", &frame); // Extracts frame number.
            cout << "\nReceived DATA " << frame << endl; // Displays received frame.

            if (frame >= 0 && frame < TOTAL_FRAMES) // Accepts valid frame numbers 0 to 7.
            {
                if (received[frame] == 0) // Checks whether this is a new frame.
                {
                    received[frame] = 1; // Marks frame as received/buffered.
                    cout << "Frame " << frame << " accepted and buffered\n"; // Reports buffering.

                    int ackNumber = frame + 1; // Calculates ACK number.
                    sprintf(buffer, "ACK:%d", ackNumber); // Creates ACK message.
                    sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)&sender, length); // Sends ACK.
                    cout << "Sent ACK " << ackNumber << endl; // Displays ACK.
                }

                while (expected < TOTAL_FRAMES && received[expected] == 1) // Delivers consecutive received frames.
                {
                    cout << "Frame " << expected << " delivered\n"; // Displays delivered frame.
                    expected++; // Advances delivery point.
                }
            }
        }
    }

    cout << "\n=====================================\n"; // Prints separator.
    cout << "All frames received successfully.\n"; // Reports successful reception.
    cout << "=====================================\n"; // Prints separator.
    close(sock); // Closes socket.
    return 0; // Ends successfully.
}
