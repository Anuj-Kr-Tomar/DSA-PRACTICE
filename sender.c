#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>

#define PORT 9000
#define SENDER_PORT 9001

#define TOTAL_FRAMES 8
#define WINDOW_SIZE 4

int main()
{
    int sock;
    int choice;
    int base = 0;
    int nextFrame = 0;
    int lostFrame;
    int lossDone = 0;
    int ack;

    char message[50];
    char buffer[50];

    struct sockaddr_in sender, receiver;
    socklen_t length = sizeof(receiver);

    // Create UDP socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    // Sender address
    sender.sin_family = AF_INET;
    sender.sin_port = htons(SENDER_PORT);
    sender.sin_addr.s_addr = inet_addr("127.0.0.7");

    // Bind sender socket
    bind(sock, (struct sockaddr *)&sender, sizeof(sender));

    // Receiver address
    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(PORT);
    receiver.sin_addr.s_addr = inet_addr("127.0.0.7");

    printf("===== SLIDING WINDOW SENDER =====\n");

    printf("\n1. Go Back N");
    printf("\n2. Selective Repeat");
    printf("\nEnter choice: ");
    scanf("%d", &choice);

    // Send selected protocol to receiver
    if(choice == 1)
        strcpy(message, "GBN");
    else
        strcpy(message, "SR");

    sendto(sock, message, strlen(message), 0,
           (struct sockaddr *)&receiver, length);

    printf("\nProtocol: %s\n", message);

    // Random frame which will be lost
    srand(time(NULL));
    lostFrame = rand() % TOTAL_FRAMES;

    printf("Random frame selected for loss: %d\n", lostFrame);

    /*
       ============================
       GO BACK N
       ============================
    */
    if(choice == 1)
    {
        printf("\n--- Go Back N Transmission ---\n");

        while(base < TOTAL_FRAMES)
        {
            // Send frames in the window
            while(nextFrame < base + WINDOW_SIZE &&
                  nextFrame < TOTAL_FRAMES)
            {
                sprintf(buffer, "DATA:%d", nextFrame);

                // Simulate loss only once
                if(nextFrame == lostFrame && lossDone == 0)
                {
                    printf("Frame %d lost\n", nextFrame);
                    lossDone = 1;
                }
                else
                {
                    sendto(sock, buffer, strlen(buffer), 0,
                           (struct sockaddr *)&receiver, length);

                    printf("Sent Frame %d\n", nextFrame);
                }

                nextFrame++;
            }

            // Set timeout
            struct timeval timeout;
            timeout.tv_sec = 2;
            timeout.tv_usec = 0;

            setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
                       &timeout, sizeof(timeout));

            // Wait for ACK
            int n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                             (struct sockaddr *)&receiver, &length);

            if(n > 0)
            {
                buffer[n] = '\0';

                sscanf(buffer, "ACK:%d", &ack);

                printf("Received ACK %d\n", ack);

                if(ack >= base)
                    base = ack + 1;
            }
            else
            {
                // Timeout - Go Back N retransmission
                printf("\nTimeout occurred\n");
                printf("Retransmitting frames from %d\n", base);

                for(int i = base; i < nextFrame; i++)
                {
                    sprintf(buffer, "DATA:%d", i);

                    sendto(sock, buffer, strlen(buffer), 0,
                           (struct sockaddr *)&receiver, length);

                    printf("Retransmitted Frame %d\n", i);
                }
            }
        }
    }

    /*
       ============================
       SELECTIVE REPEAT
       ============================
    */
    else if(choice == 2)
    {
        int acknowledged[TOTAL_FRAMES] = {0};
        int sent[TOTAL_FRAMES] = {0};

        printf("\n--- Selective Repeat Transmission ---\n");

        while(base < TOTAL_FRAMES)
        {
            // Send frames in current window
            for(int i = base;
                i < base + WINDOW_SIZE && i < TOTAL_FRAMES;
                i++)
            {
                if(sent[i] == 0)
                {
                    sprintf(buffer, "DATA:%d", i);

                    // Simulate one lost frame
                    if(i == lostFrame && lossDone == 0)
                    {
                        printf("Frame %d lost\n", i);
                        lossDone = 1;
                    }
                    else
                    {
                        sendto(sock, buffer, strlen(buffer), 0,
                               (struct sockaddr *)&receiver, length);

                        printf("Sent Frame %d\n", i);
                    }

                    sent[i] = 1;
                }
            }

            // Set timeout
            struct timeval timeout;
            timeout.tv_sec = 2;
            timeout.tv_usec = 0;

            setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
                       &timeout, sizeof(timeout));

            // Wait for ACK
            int n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                             (struct sockaddr *)&receiver, &length);

            if(n > 0)
            {
                buffer[n] = '\0';

                sscanf(buffer, "ACK:%d", &ack);

                printf("Received ACK %d\n", ack);

                acknowledged[ack] = 1;

                // Move window forward
                while(base < TOTAL_FRAMES &&
                      acknowledged[base] == 1)
                {
                    base++;
                }
            }
            else
            {
                // Timeout - retransmit only unacknowledged frame
                printf("\nTimeout occurred\n");

                for(int i = base;
                    i < base + WINDOW_SIZE && i < TOTAL_FRAMES;
                    i++)
                {
                    if(acknowledged[i] == 0)
                    {
                        sprintf(buffer, "DATA:%d", i);

                        sendto(sock, buffer, strlen(buffer), 0,
                               (struct sockaddr *)&receiver, length);

                        printf("Retransmitted Frame %d\n", i);
                    }
                }
            }
        }
    }

    printf("\nAll frames transmitted successfully.\n");

    close(sock);

    return 0;
}
