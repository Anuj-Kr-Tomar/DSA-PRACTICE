#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 9000
#define TOTAL_FRAMES 8

int main()
{
    int sock;
    int expected = 0;
    int received[TOTAL_FRAMES] = {0};

    char buffer[50];
    char protocol[10];

    struct sockaddr_in receiver, sender;
    socklen_t length = sizeof(sender);

    // Create UDP socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    // Receiver address
    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(PORT);
    receiver.sin_addr.s_addr = inet_addr("127.0.0.7");

    // Bind receiver socket
    bind(sock, (struct sockaddr *)&receiver, sizeof(receiver));

    printf("===== SLIDING WINDOW RECEIVER =====\n");

    printf("\nWaiting for sender...\n");

    // Receive protocol type
    int n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                     (struct sockaddr *)&sender, &length);

    buffer[n] = '\0';

    strcpy(protocol, buffer);

    printf("\nProtocol: %s\n", protocol);

    /*
       ============================
       GO BACK N
       ============================
    */
    if(strcmp(protocol, "GBN") == 0)
    {
        printf("\n--- Go Back N Receiver ---\n");

        while(expected < TOTAL_FRAMES)
        {
            n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                         (struct sockaddr *)&sender, &length);

            buffer[n] = '\0';

            int frame;

            sscanf(buffer, "DATA:%d", &frame);

            printf("Received Frame %d\n", frame);

            if(frame == expected)
            {
                printf("Frame %d accepted\n", frame);

                // Send ACK
                sprintf(buffer, "ACK:%d", frame);

                sendto(sock, buffer, strlen(buffer), 0,
                       (struct sockaddr *)&sender, length);

                expected++;
            }
            else
            {
                // Out-of-order frame is discarded
                printf("Frame %d discarded\n", frame);

                // ACK last correctly received frame
                sprintf(buffer, "ACK:%d", expected - 1);

                sendto(sock, buffer, strlen(buffer), 0,
                       (struct sockaddr *)&sender, length);
            }
        }
    }

    /*
       ============================
       SELECTIVE REPEAT
       ============================
    */
    else if(strcmp(protocol, "SR") == 0)
    {
        printf("\n--- Selective Repeat Receiver ---\n");

        while(expected < TOTAL_FRAMES)
        {
            n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                         (struct sockaddr *)&sender, &length);

            buffer[n] = '\0';

            int frame;

            sscanf(buffer, "DATA:%d", &frame);

            printf("Received Frame %d\n", frame);

            if(received[frame] == 0)
            {
                // Store received frame
                received[frame] = 1;

                printf("Frame %d accepted and buffered\n",
                       frame);

                // Send ACK for this particular frame
                sprintf(buffer, "ACK:%d", frame);

                sendto(sock, buffer, strlen(buffer), 0,
                       (struct sockaddr *)&sender, length);
            }

            // Deliver frames in order
            while(expected < TOTAL_FRAMES &&
                  received[expected] == 1)
            {
                printf("Frame %d delivered\n", expected);

                expected++;
            }
        }
    }

    printf("\nAll frames received successfully.\n");

    close(sock);

    return 0;
}
