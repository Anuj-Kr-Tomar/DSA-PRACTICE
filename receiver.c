#include <stdio.h>      // Used for printf()
#include <stdlib.h>     // General utility functions
#include <string.h>     // Used for strcpy(), strcmp(), strlen()
#include <arpa/inet.h>  // Used for socket address functions
#include <unistd.h>     // Used for close()


// -----------------------------------------
// CONSTANTS
// -----------------------------------------

// Receiver will listen on port 9000.
#define PORT 9000

// Total number of frames expected.
#define TOTAL_FRAMES 8


int main()
{
    // -----------------------------------------
    // VARIABLE DECLARATIONS
    // -----------------------------------------

    // Socket descriptor.
    // It is used for communication.
    int sock;


    // 'expected' represents the next frame
    // that the receiver is expecting in order.
    //
    // Initially receiver expects frame 0.
    int expected = 0;


    // This array is mainly used for
    // Selective Repeat.
    //
    // received[i] tells whether frame i
    // has been received.
    //
    // 0 -> not received
    // 1 -> received
    int received[TOTAL_FRAMES] = {0};


    // Buffer stores received messages.
    //
    // Examples:
    // "GBN"
    // "SR"
    // "DATA:2"
    char buffer[50];


    // Stores the selected protocol.
    //
    // It will contain:
    // "GBN" or "SR"
    char protocol[10];


    // Structure for receiver's IP address and port.
    struct sockaddr_in receiver;


    // Structure used to store sender's address.
    struct sockaddr_in sender;


    // Stores size of sender address.
    socklen_t length = sizeof(sender);


    // -----------------------------------------
    // CREATE UDP SOCKET
    // -----------------------------------------

    // Create a UDP socket.
    //
    // AF_INET    -> IPv4
    // SOCK_DGRAM -> UDP
    // 0          -> default UDP protocol
    sock = socket(AF_INET, SOCK_DGRAM, 0);


    // -----------------------------------------
    // SET RECEIVER ADDRESS
    // -----------------------------------------

    // Use IPv4.
    receiver.sin_family = AF_INET;


    // Receiver will listen on port 9000.
    receiver.sin_port = htons(PORT);


    // Receiver is running on local machine.
    receiver.sin_addr.s_addr = inet_addr("127.0.0.7");


    // -----------------------------------------
    // BIND RECEIVER SOCKET
    // -----------------------------------------

    // bind() connects the socket with
    // receiver's IP address and port.
    //
    // After this, receiver can listen
    // for incoming UDP packets on port 9000.
    bind(sock,
         (struct sockaddr *)&receiver,
         sizeof(receiver));


    // -----------------------------------------
    // DISPLAY PROJECT TITLE
    // -----------------------------------------

    printf("=====================================\n");
    printf("       SLIDING WINDOW RECEIVER       \n");
    printf("=====================================\n");


    printf("\nWaiting for sender...\n");


    // -----------------------------------------
    // RECEIVE PROTOCOL NAME
    // -----------------------------------------

    // First, receiver waits for sender to tell
    // which protocol is being used.
    //
    // Sender sends either:
    //
    // "GBN"
    //
    // OR
    //
    // "SR"
    int n = recvfrom(sock,
                     buffer,
                     sizeof(buffer) - 1,
                     0,
                     (struct sockaddr *)&sender,
                     &length);


    // Add null character at the end
    // to make buffer a proper C string.
    buffer[n] = '\0';


    // Copy received protocol into protocol variable.
    strcpy(protocol, buffer);


    printf("\nProtocol: %s\n", protocol);


    // =================================================
    //                 GO-BACK-N
    // =================================================

    // Check whether sender selected GBN.
    if(strcmp(protocol, "GBN") == 0)
    {
        printf("\n--- Go Back N Receiver ---\n");


        // Continue until all 8 frames are received
        // in correct order.
        while(expected < TOTAL_FRAMES)
        {

            // -----------------------------------------
            // RECEIVE FRAME
            // -----------------------------------------

            // Wait for a frame from sender.
            n = recvfrom(sock,
                         buffer,
                         sizeof(buffer) - 1,
                         0,
                         (struct sockaddr *)&sender,
                         &length);


            // Add null character at the end.
            buffer[n] = '\0';


            // Variable to store frame number.
            int frame;


            // Extract frame number from message.
            //
            // Example:
            // "DATA:3"
            //
            // frame becomes 3.
            sscanf(buffer, "DATA:%d", &frame);


            printf("Received Frame %d\n", frame);


            // -----------------------------------------
            // CHECK WHETHER FRAME IS EXPECTED
            // -----------------------------------------

            // If received frame is exactly the frame
            // we were expecting...
            if(frame == expected)
            {

                printf("Frame %d accepted\n", frame);


                // Create ACK message.
                //
                // Example:
                // Frame 3
                // ACK = "ACK:3"
                sprintf(buffer, "ACK:%d", frame);


                // Send ACK back to sender.
                sendto(sock,
                       buffer,
                       strlen(buffer),
                       0,
                       (struct sockaddr *)&sender,
                       length);


                // Now receiver expects the next frame.
                //
                // Example:
                // expected = 3
                // after accepting frame 3:
                // expected = 4
                expected++;
            }


            // -----------------------------------------
            // OUT-OF-ORDER FRAME
            // -----------------------------------------

            else
            {
                // The received frame is not the frame
                // we are currently expecting.
                //
                // In Go-Back-N, out-of-order frames
                // are discarded.
                printf("Frame %d discarded\n", frame);


                // Send ACK for the last correctly
                // received frame.
                //
                // Example:
                // expected = 4
                //
                // Receiver got frame 6.
                // Frame 4 is missing.
                //
                // Receiver sends:
                // ACK:3
                //
                // This tells sender that frame 3
                // was the last correctly received
                // frame in order.
                sprintf(buffer, "ACK:%d", expected - 1);


                // Send ACK to sender.
                sendto(sock,
                       buffer,
                       strlen(buffer),
                       0,
                       (struct sockaddr *)&sender,
                       length);
            }
        }
    }


    // =================================================
    //              SELECTIVE REPEAT
    // =================================================

    // Check whether sender selected SR.
    else if(strcmp(protocol, "SR") == 0)
    {
        printf("\n--- Selective Repeat Receiver ---\n");


        // Continue until all frames have been
        // received and delivered.
        while(expected < TOTAL_FRAMES)
        {

            // -----------------------------------------
            // RECEIVE FRAME
            // -----------------------------------------

            n = recvfrom(sock,
                         buffer,
                         sizeof(buffer) - 1,
                         0,
                         (struct sockaddr *)&sender,
                         &length);


            // Add null character to the received string.
            buffer[n] = '\0';


            // Variable to store received frame number.
            int frame;


            // Extract frame number.
            //
            // Example:
            // "DATA:5"
            //
            // frame becomes 5.
            sscanf(buffer, "DATA:%d", &frame);


            printf("Received Frame %d\n", frame);


            // -----------------------------------------
            // CHECK WHETHER FRAME IS ALREADY RECEIVED
            // -----------------------------------------

            // If this frame has not been received before...
            if(received[frame] == 0)
            {

                // Mark this frame as received.
                received[frame] = 1;


                // In Selective Repeat,
                // even an out-of-order frame is accepted
                // and stored/buffered.
                printf("Frame %d accepted and buffered\n",
                       frame);


                // Create ACK for this exact frame.
                //
                // Example:
                // Frame 5
                // ACK:5
                sprintf(buffer, "ACK:%d", frame);


                // Send ACK back to sender.
                sendto(sock,
                       buffer,
                       strlen(buffer),
                       0,
                       (struct sockaddr *)&sender,
                       length);
            }


            // -----------------------------------------
            // DELIVER FRAMES IN ORDER
            // -----------------------------------------

            // Now check whether the frame we are
            // currently expecting has been received.
            //
            // If yes, we can deliver it.
            //
            // We continue moving forward while
            // consecutive frames are available.
            while(expected < TOTAL_FRAMES &&
                  received[expected] == 1)
            {

                printf("Frame %d delivered\n", expected);


                // Move to next expected frame.
                expected++;
            }
        }
    }


    // -----------------------------------------
    // ALL FRAMES RECEIVED
    // -----------------------------------------

    printf("\nAll frames received successfully.\n");


    // Close the socket.
    close(sock);


    // End program successfully.
    return 0;
}
