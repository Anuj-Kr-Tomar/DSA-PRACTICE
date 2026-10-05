#include <iostream>       // Provides cin, cout and endl.
#include <cstdlib>        // Provides general utility functions.
#include <cstring>        // Provides strcpy(), strlen(), etc.
#include <arpa/inet.h>    // Provides IPv4 socket/network functions.
#include <unistd.h>       // Provides close().
#include <sys/time.h>     // Provides struct timeval for timeout.

using namespace std;      // Lets us use cin/cout without std::.

#define PORT 9000         // Receiver UDP port.
#define SENDER_PORT 9001  // Sender UDP port.
#define TOTAL_FRAMES 8    // Total number of frames.
#define WINDOW_SIZE 4     // Sliding-window size.
#define LOST_FRAME 4      // Frame 4 is intentionally lost once.

int main()                  // Program starts here.
{
    int sock;               // Stores socket descriptor.
    int choice;             // Stores 1 for GBN or 2 for SR.
    char message[50];       // Stores protocol message.
    char buffer[50];        // Stores DATA/ACK messages.
    struct sockaddr_in sender{}, receiver{}; // Stores sender/receiver addresses.
    socklen_t length = sizeof(receiver); // Stores receiver address size.

    // CREATE SOCKET
    sock = socket(AF_INET, SOCK_DGRAM, 0); // Creates IPv4 UDP socket.

    if (sock < 0) // Checks socket creation failure.
    {
        perror("Socket creation failed"); // Prints system error.
        return 1; // Exits with error.
    }

    // SENDER ADDRESS
    sender.sin_family = AF_INET; // Uses IPv4.
    sender.sin_port = htons(SENDER_PORT); // Converts port 9001 to network byte order.
    sender.sin_addr.s_addr = inet_addr("127.0.0.7"); // Sets sender IP.

    if (bind(sock, (struct sockaddr *)&sender, sizeof(sender)) < 0) // Binds sender socket.
    {
        perror("Sender bind failed"); // Prints bind error.
        close(sock); // Closes socket.
        return 1; // Exits.
    }

    // RECEIVER ADDRESS
    receiver.sin_family = AF_INET; // Uses IPv4 for receiver.
    receiver.sin_port = htons(PORT); // Sets receiver port 9000.
    receiver.sin_addr.s_addr = inet_addr("127.0.0.7"); // Sets receiver IP.

    cout << "=====================================\n"; // Prints separator.
    cout << " SLIDING WINDOW SENDER\n"; // Prints sender heading.
    cout << "=====================================\n"; // Prints separator.
    cout << "Roll Number : 7\n"; // Displays roll number.
    cout << "Sender IP : 127.0.0.7\n"; // Displays sender IP.
    cout << "Sender Port : 9001\n"; // Displays sender port.
    cout << "\n1. Go Back N"; // Displays GBN option.
    cout << "\n2. Selective Repeat"; // Displays SR option.
    cout << "\nEnter choice: "; // Prompts for choice.
    cin >> choice; // Reads choice.

    if (choice != 1 && choice != 2) // Checks invalid choice.
    {
        cout << "Invalid choice.\n"; // Displays error.
        close(sock); // Closes socket.
        return 1; // Exits.
    }

    // SEND PROTOCOL
    if (choice == 1) // Checks for GBN.
        strcpy(message, "GBN"); // Stores GBN in message.
    else // Otherwise SR was selected.
        strcpy(message, "SR"); // Stores SR in message.

    sendto(sock, message, strlen(message), 0, (struct sockaddr *)&receiver, length); // Sends protocol to receiver.
    cout << "\nProtocol: " << message << endl; // Displays selected protocol.
    cout << "Frame " << LOST_FRAME << " will be lost once.\n"; // Announces simulated loss.

    // GO BACK N
    if (choice == 1) // Runs GBN sender.
    {
        int base = 0; // First unacknowledged frame.
        int nextFrame = 0; // Next frame to send.
        bool lossDone = false; // Ensures Frame 4 is lost only once.

        cout << "\n--- GO BACK N TRANSMISSION ---\n"; // Prints GBN heading.

        while (base < TOTAL_FRAMES) // Continues until all frames are acknowledged.
        {
            while (nextFrame < base + WINDOW_SIZE && nextFrame < TOTAL_FRAMES) // Sends frames inside current window.
            {
                sprintf(buffer, "DATA:%d", nextFrame); // Creates DATA:n message.

                if (nextFrame == LOST_FRAME && !lossDone) // Checks whether Frame 4 should be lost.
                {
                    cout << "Frame " << nextFrame << " LOST\n"; // Displays simulated loss.
                    lossDone = true; // Records that loss has happened.
                }
                else // Sends all other frames normally.
                {
                    sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)&receiver, length); // Sends DATA frame.
                    cout << "Sent DATA " << nextFrame << endl; // Displays sent frame.
                }

                nextFrame++; // Moves to next frame.
            }

            // SET TIMEOUT
            struct timeval timeout; // Stores timeout duration.
            timeout.tv_sec = 2; // Sets timeout to 2 seconds.
            timeout.tv_usec = 0; // Sets microseconds to zero.

            setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)); // Sets receive timeout.

            // WAIT FOR ACK
            int n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&receiver, &length); // Waits for ACK.

            if (n > 0) // Executes when an ACK arrives.
            {
                buffer[n] = '\0'; // Terminates ACK string.
                int ack; // Stores ACK number.
                sscanf(buffer, "ACK:%d", &ack); // Extracts ACK number.
                cout << "Received ACK " << ack << endl; // Displays ACK.

                if (ack > base) // Checks whether ACK advances the window.
                {
                    base = ack; // Moves window base forward.
                }
            }
            else // Executes when ACK times out.
            {
                cout << "\nTIMEOUT OCCURRED\n"; // Reports timeout.
                cout << "Go Back N: retransmitting from Frame " << base << endl; // Reports GBN retransmission start.

                for (int i = base; i < nextFrame; i++) // Loops through all sent frames from base onward.
                {
                    sprintf(buffer, "DATA:%d", i); // Recreates DATA:i message.
                    sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)&receiver, length); // Retransmits frame.
                    cout << "Retransmitted DATA " << i << endl; // Displays retransmission.
                }
            }
        }
    }

    // SELECTIVE REPEAT
    else // Runs SR sender.
    {
        int base = 0; // First frame not yet cleared from window.
        bool sent[TOTAL_FRAMES] = {false}; // Tracks whether each frame was sent.
        bool acknowledged[TOTAL_FRAMES] = {false}; // Tracks whether each frame was acknowledged.
        bool lossDone = false; // Ensures Frame 4 is lost only once.

        cout << "\n--- SELECTIVE REPEAT TRANSMISSION ---\n"; // Prints SR heading.

        while (base < TOTAL_FRAMES) // Continues until all frames are acknowledged.
        {
            for (int i = base; i < base + WINDOW_SIZE && i < TOTAL_FRAMES; i++) // Loops through current window.
            {
                if (!sent[i]) // Sends only frames not already attempted.
                {
                    sprintf(buffer, "DATA:%d", i); // Creates DATA:i message.

                    if (i == LOST_FRAME && !lossDone) // Checks whether Frame 4 should be lost.
                    {
                        cout << "Frame " << i << " LOST\n"; // Displays simulated loss.
                        lossDone = true; // Records that loss has happened.
                    }
                    else // Sends every other frame.
                    {
                        sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)&receiver, length); // Sends DATA frame.
                        cout << "Sent DATA " << i << endl; // Displays sent frame.
                    }

                    sent[i] = true; // Marks frame as sent/attempted.
                }
            }

            // SET TIMEOUT
            struct timeval timeout; // Stores timeout duration.
            timeout.tv_sec = 2; // Sets timeout to 2 seconds.
            timeout.tv_usec = 0; // Sets microseconds to zero.

            setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)); // Sets receive timeout.

            // WAIT FOR ACK
            int n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&receiver, &length); // Waits for ACK.

            if (n > 0) // Executes when ACK is received.
            {
                buffer[n] = '\0'; // Terminates ACK string.
                int ack; // Stores ACK number.
                sscanf(buffer, "ACK:%d", &ack); // Extracts ACK number.
                cout << "Received ACK " << ack << endl; // Displays ACK.

                int frame = ack - 1; // Converts ACK number into corresponding frame number.

                if (frame >= 0 && frame < TOTAL_FRAMES) // Checks whether frame index is valid.
                {
                    acknowledged[frame] = true; // Marks that frame as acknowledged.
                }

                while (base < TOTAL_FRAMES && acknowledged[base]) // Moves window while base frames are acknowledged.
                {
                    base++; // Slides window forward.
                }
            }
            else // Executes when timeout occurs.
            {
                cout << "\nTIMEOUT OCCURRED\n"; // Reports timeout.
                cout << "Selective Repeat: retransmitting only unacknowledged frames\n"; // Explains SR behavior.

                for (int i = base; i < base + WINDOW_SIZE && i < TOTAL_FRAMES; i++) // Checks frames in current window.
                {
                    if (!acknowledged[i]) // Selects only unacknowledged frames.
                    {
                        sprintf(buffer, "DATA:%d", i); // Creates DATA:i message.
                        sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)&receiver, length); // Retransmits only this frame.
                        cout << "Retransmitted DATA " << i << endl; // Displays retransmission.
                    }
                }
            }
        }
    }

    cout << "\n=====================================\n"; // Prints separator.
    cout << "All frames transmitted successfully.\n"; // Reports successful transmission.
    cout << "=====================================\n"; // Prints separator.
    close(sock); // Closes UDP socket.
    return 0; // Ends successfully.
}
