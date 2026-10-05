#include <stdio.h>      // Used for printf(), scanf()
#include <stdlib.h>     // Used for rand(), srand()
#include <string.h>     // Used for strcpy(), strlen()
#include <time.h>       // Used for time()
#include <arpa/inet.h>  // Used for socket address functions
#include <unistd.h>     // Used for close()
#include <sys/time.h>   // Used for struct timeval and timeout

// Port number on which receiver is running
#define PORT 9000

// Port number used by sender
#define SENDER_PORT 9001

// Total number of frames that we want to send
// Frames will be: 0, 1, 2, 3, 4, 5, 6, 7
#define TOTAL_FRAMES 8

// Maximum number of frames that can be sent
// at one time in the sliding window
#define WINDOW_SIZE 4


int main()
{
    // -----------------------------------------
    // VARIABLE DECLARATIONS
    // -----------------------------------------

    // 'sock' stores the socket number.
    // Socket is used for sending and receiving data.
    int sock;

    // User will select:
    // 1 -> Go-Back-N
    // 2 -> Selective Repeat
    int choice;

    // 'base' represents the first unacknowledged frame.
    // It is also the beginning of our current window.
    int base = 0;

    // 'nextFrame' represents the next frame
    // which the sender has to send.
    int nextFrame = 0;

    // This variable stores the randomly selected
    // frame which we will intentionally "lose".
    int lostFrame;

    // This variable makes sure that we simulate
    // loss only once.
    int lossDone = 0;

    // Stores the ACK number received from receiver.
    int ack;

    // Used to create messages such as:
    // "GBN", "SR", "DATA:0", "ACK:2"
    char message[50];

    // Temporary buffer used for sending and receiving messages.
    char buffer[50];

    // Structure containing sender's IP address and port.
    struct sockaddr_in sender;

    // Structure containing receiver's IP address and port.
    struct sockaddr_in receiver;

    // Stores the size of receiver address structure.
    socklen_t length = sizeof(receiver);


    // -----------------------------------------
    // CREATE UDP SOCKET
    // -----------------------------------------

    // socket() creates a communication endpoint.
    //
    // AF_INET     -> IPv4
    // SOCK_DGRAM  -> UDP
    // 0           -> default protocol for UDP
    //
    // The returned value is stored in 'sock'.
    sock = socket(AF_INET, SOCK_DGRAM, 0);


    // -----------------------------------------
    // SET SENDER ADDRESS
    // -----------------------------------------

    // We are using IPv4.
    sender.sin_family = AF_INET;

    // Set sender port to 9001.
    //
    // htons() converts the port number from
    // computer's byte order to network byte order.
    sender.sin_port = htons(SENDER_PORT);

    // Set sender IP address.
    //
    // 127.0.0.7 is a loopback address.
    // It allows sender and receiver to run
    // on the same computer.
    sender.sin_addr.s_addr = inet_addr("127.0.0.7");


    // -----------------------------------------
    // BIND SENDER SOCKET
    // -----------------------------------------

    // bind() attaches our socket to the
    // specified IP address and port.
    //
    // Here sender will use:
    // IP   -> 127.0.0.7
    // Port -> 9001
    bind(sock,
         (struct sockaddr *)&sender,
         sizeof(sender));


    // -----------------------------------------
    // SET RECEIVER ADDRESS
    // -----------------------------------------

    // Receiver also uses IPv4.
    receiver.sin_family = AF_INET;

    // Receiver is listening on port 9000.
    receiver.sin_port = htons(PORT);

    // Receiver is running on the same machine.
    receiver.sin_addr.s_addr = inet_addr("127.0.0.7");


    // -----------------------------------------
    // DISPLAY PROJECT TITLE
    // -----------------------------------------

    printf("=====================================\n");
    printf("       SLIDING WINDOW SENDER        \n");
    printf("=====================================\n");


    // -----------------------------------------
    // ASK USER TO SELECT PROTOCOL
    // -----------------------------------------

    printf("\n1. Go Back N");
    printf("\n2. Selective Repeat");
    printf("\nEnter choice: ");

    // Read user's choice.
    scanf("%d", &choice);


    // -----------------------------------------
    // SELECT PROTOCOL
    // -----------------------------------------

    // If user selects 1,
    // store "GBN" in message.
    if(choice == 1)
        strcpy(message, "GBN");

    // Otherwise store "SR".
    else
        strcpy(message, "SR");


    // -----------------------------------------
    // SEND PROTOCOL NAME TO RECEIVER
    // -----------------------------------------

    // Before sending frames, sender tells receiver
    // which protocol we are going to use.
    //
    // Example:
    // If user selected 1:
    //       message = "GBN"
    //
    // If user selected 2:
    //       message = "SR"
    sendto(sock,
           message,
           strlen(message),
           0,
           (struct sockaddr *)&receiver,
           length);


    printf("\nProtocol: %s\n", message);


    // -----------------------------------------
    // SELECT A RANDOM FRAME TO BE LOST
    // -----------------------------------------

    // srand() initializes the random number generator.
    // time(NULL) makes the random value different
    // each time the program runs.
    srand(time(NULL));

    // rand() % TOTAL_FRAMES gives a number
    // between 0 and 7.
    //
    // That frame will be intentionally skipped
    // to simulate packet/frame loss.
    lostFrame = rand() % TOTAL_FRAMES;

    printf("Random frame selected for loss: %d\n", lostFrame);


    // =================================================
    //                 GO-BACK-N
    // =================================================

    // If user selected Go-Back-N
    if(choice == 1)
    {
        printf("\n--- Go Back N Transmission ---\n");


        // Continue until all frames are acknowledged.
        //
        // base tells us the first frame that
        // has not been acknowledged yet.
        while(base < TOTAL_FRAMES)
        {

            // -----------------------------------------
            // SEND FRAMES INSIDE CURRENT WINDOW
            // -----------------------------------------

            // We can send at most WINDOW_SIZE frames.
            //
            // Example:
            // Window size = 4
            //
            // First window:
            // 0 1 2 3
            //
            // Then:
            // 4 5 6 7
            while(nextFrame < base + WINDOW_SIZE &&
                  nextFrame < TOTAL_FRAMES)
            {

                // Create the data message.
                //
                // Example:
                // Frame 2 becomes:
                // "DATA:2"
                sprintf(buffer, "DATA:%d", nextFrame);


                // -----------------------------------------
                // SIMULATE FRAME LOSS
                // -----------------------------------------

                // If current frame is the randomly
                // selected lost frame AND we have not
                // simulated loss before...
                if(nextFrame == lostFrame &&
                   lossDone == 0)
                {
                    // We DO NOT call sendto().
                    //
                    // Therefore the frame is not actually
                    // sent to the receiver.
                    //
                    // This behaves like a lost frame.
                    printf("Frame %d lost\n", nextFrame);

                    // Mark that loss simulation is completed.
                    lossDone = 1;
                }

                else
                {
                    // Send the frame to receiver.
                    sendto(sock,
                           buffer,
                           strlen(buffer),
                           0,
                           (struct sockaddr *)&receiver,
                           length);

                    printf("Sent Frame %d\n", nextFrame);
                }


                // Move to the next frame.
                nextFrame++;
            }


            // -----------------------------------------
            // SET TIMEOUT
            // -----------------------------------------

            // struct timeval is used to define
            // how long we should wait for an ACK.
            struct timeval timeout;

            // Wait for 2 seconds.
            timeout.tv_sec = 2;

            // Additional microseconds = 0.
            timeout.tv_usec = 0;


            // SO_RCVTIMEO sets a timeout for receiving data.
            //
            // If ACK does not arrive within 2 seconds,
            // recvfrom() will return an error/timeout.
            setsockopt(sock,
                       SOL_SOCKET,
                       SO_RCVTIMEO,
                       &timeout,
                       sizeof(timeout));


            // -----------------------------------------
            // WAIT FOR ACK
            // -----------------------------------------

            // Wait for ACK from receiver.
            //
            // recvfrom() receives a UDP message
            // and also gives the sender's address.
            int n = recvfrom(sock,
                             buffer,
                             sizeof(buffer) - 1,
                             0,
                             (struct sockaddr *)&receiver,
                             &length);


            // If n > 0, an ACK was received.
            if(n > 0)
            {
                // Add '\0' at the end so that buffer
                // becomes a proper C string.
                buffer[n] = '\0';


                // Extract ACK number.
                //
                // Example:
                // "ACK:2"
                //
                // ack becomes 2.
                sscanf(buffer, "ACK:%d", &ack);


                printf("Received ACK %d\n", ack);


                // -----------------------------------------
                // MOVE SLIDING WINDOW
                // -----------------------------------------

                // If ACK number is greater than or equal
                // to current base, move base forward.
                //
                // Example:
                // base = 0
                // ACK = 2
                //
                // Then:
                // base = 3
                //
                // Frames 0,1,2 are now acknowledged.
                if(ack >= base)
                    base = ack + 1;
            }

            // -----------------------------------------
            // TIMEOUT OCCURRED
            // -----------------------------------------

            else
            {
                printf("\nTimeout occurred\n");

                printf("Retransmitting frames from %d\n",
                       base);


                // -----------------------------------------
                // RETRANSMIT ALL FRAMES
                // -----------------------------------------

                // In Go-Back-N, if timeout occurs,
                // we retransmit ALL frames from base
                // to nextFrame - 1.
                //
                // This is the main feature of Go-Back-N.
                for(int i = base; i < nextFrame; i++)
                {

                    // Create frame message.
                    //
                    // Example:
                    // "DATA:3"
                    sprintf(buffer, "DATA:%d", i);


                    // Send the frame again.
                    sendto(sock,
                           buffer,
                           strlen(buffer),
                           0,
                           (struct sockaddr *)&receiver,
                           length);


                    printf("Retransmitted Frame %d\n", i);
                }
            }
        }
    }


    // =================================================
    //              SELECTIVE REPEAT
    // =================================================

    // If user selected Selective Repeat
    else if(choice == 2)
    {
        // -----------------------------------------
        // ACKNOWLEDGEMENT ARRAY
        // -----------------------------------------

        // acknowledged[i] tells whether frame i
        // has received an ACK.
        //
        // 0 -> not acknowledged
        // 1 -> acknowledged
        int acknowledged[TOTAL_FRAMES] = {0};


        // -----------------------------------------
        // SENT ARRAY
        // -----------------------------------------

        // sent[i] tells whether frame i
        // has already been sent.
        //
        // 0 -> not sent
        // 1 -> already sent
        int sent[TOTAL_FRAMES] = {0};


        printf("\n--- Selective Repeat Transmission ---\n");


        // Continue until all frames are acknowledged.
        while(base < TOTAL_FRAMES)
        {

            // -----------------------------------------
            // SEND CURRENT WINDOW
            // -----------------------------------------

            // Send frames from base to
            // base + WINDOW_SIZE - 1.
            for(int i = base;
                i < base + WINDOW_SIZE &&
                i < TOTAL_FRAMES;
                i++)
            {

                // Send only if this frame has
                // not already been sent.
                if(sent[i] == 0)
                {

                    // Create data message.
                    //
                    // Example:
                    // "DATA:4"
                    sprintf(buffer, "DATA:%d", i);


                    // -----------------------------------------
                    // SIMULATE FRAME LOSS
                    // -----------------------------------------

                    if(i == lostFrame &&
                       lossDone == 0)
                    {
                        // Do not call sendto().
                        // So the frame is intentionally lost.
                        printf("Frame %d lost\n", i);

                        // Loss is simulated only once.
                        lossDone = 1;
                    }

                    else
                    {
                        // Send frame normally.
                        sendto(sock,
                               buffer,
                               strlen(buffer),
                               0,
                               (struct sockaddr *)&receiver,
                               length);

                        printf("Sent Frame %d\n", i);
                    }


                    // Mark frame as sent.
                    sent[i] = 1;
                }
            }


            // -----------------------------------------
            // SET TIMEOUT
            // -----------------------------------------

            struct timeval timeout;

            // Wait for 2 seconds for ACK.
            timeout.tv_sec = 2;
            timeout.tv_usec = 0;


            setsockopt(sock,
                       SOL_SOCKET,
                       SO_RCVTIMEO,
                       &timeout,
                       sizeof(timeout));


            // -----------------------------------------
            // WAIT FOR ACK
            // -----------------------------------------

            int n = recvfrom(sock,
                             buffer,
                             sizeof(buffer) - 1,
                             0,
                             (struct sockaddr *)&receiver,
                             &length);


            // If ACK is received
            if(n > 0)
            {
                buffer[n] = '\0';


                // Extract ACK number.
                //
                // Example:
                // "ACK:5"
                //
                // ack = 5
                sscanf(buffer, "ACK:%d", &ack);


                printf("Received ACK %d\n", ack);


                // Mark this particular frame
                // as acknowledged.
                acknowledged[ack] = 1;


                // -----------------------------------------
                // MOVE WINDOW
                // -----------------------------------------

                // Keep moving base forward while
                // consecutive frames are acknowledged.
                //
                // Example:
                //
                // acknowledged:
                // 1 1 1 0 1
                //
                // base will move until frame 3,
                // because frame 3 is not acknowledged.
                while(base < TOTAL_FRAMES &&
                      acknowledged[base] == 1)
                {
                    base++;
                }
            }


            // -----------------------------------------
            // TIMEOUT OCCURRED
            // -----------------------------------------

            else
            {
                printf("\nTimeout occurred\n");


                // -----------------------------------------
                // RETRANSMIT ONLY UNACKNOWLEDGED FRAMES
                // -----------------------------------------

                // Unlike Go-Back-N,
                // Selective Repeat does NOT retransmit
                // every frame.
                //
                // It retransmits only frames for which
                // ACK has not been received.
                for(int i = base;
                    i < base + WINDOW_SIZE &&
                    i < TOTAL_FRAMES;
                    i++)
                {

                    // If this frame has not been acknowledged
                    if(acknowledged[i] == 0)
                    {

                        // Create frame message.
                        sprintf(buffer, "DATA:%d", i);


                        // Send only this particular frame again.
                        sendto(sock,
                               buffer,
                               strlen(buffer),
                               0,
                               (struct sockaddr *)&receiver,
                               length);


                        printf("Retransmitted Frame %d\n", i);
                    }
                }
            }
        }
    }


    // -----------------------------------------
    // TRANSMISSION COMPLETED
    // -----------------------------------------

    printf("\nAll frames transmitted successfully.\n");


    // Close the socket after communication is complete.
    close(sock);


    // End program successfully.
    return 0;
}
