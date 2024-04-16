#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <Ws2tcpip.h>

#include "ChessNet.h"

#pragma comment(lib, "ws2_32.lib") // Link with ws2_32.lib

int main() {
    WSADATA wsa;
    int listenfd, connfd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed");
        exit(EXIT_FAILURE);
    }

    // Create socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set options to reuse the IP address and IP port if either is already in use
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt)) == SOCKET_ERROR) {
        perror("setsockopt(SO_REUSEADDR) failed");
        return -1;
    }

    // Bind socket to port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(listenfd, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(listenfd, 1) == SOCKET_ERROR) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    INFO("Server listening on port %d", PORT);
    // Accept incoming connection
    if ((connfd = accept(listenfd, (struct sockaddr *)&address, &addrlen)) == INVALID_SOCKET) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    INFO("Server accepted connection");

   INFO("Server accepted connection");

    //INITIALIZE GAME
    ChessGame game;
    initialize_game(&game);
    display_chessboard(&game);

    char buffer[BUFFER_SIZE] = {0};
    int sendCMD;
    int recCMD;

    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);
        int nbytes = recv(connfd, buffer, BUFFER_SIZE, 0);

        if (nbytes <= 0) 
        {
            perror("[Server] read() failed.");
            break;
        }

        printf("[Server] Received from client: %s\n", buffer);
        recCMD = receive_command(&game, buffer, connfd, false);
        if (recCMD == COMMAND_FORFEIT)
        {
            printf("[Server] Client Forfeiting...\n");
            break;
        }

        if((recCMD == COMMAND_IMPORT || recCMD == COMMAND_LOAD) && game.currentPlayer == WHITE_PLAYER)
            continue;
        else
        {
            printf("[Server] Enter message: ");
            memset(buffer, 0, BUFFER_SIZE);
            fgets(buffer, BUFFER_SIZE, stdin);
            buffer[strlen(buffer)-1] = '\0';
            sendCMD = send_command(&game, buffer, connfd, false);

            while(!(sendCMD == COMMAND_MOVE || sendCMD == COMMAND_FORFEIT))
            {

                //if((sendCMD == COMMAND_IMPORT || sendCMD == COMMAND_LOAD) && game.currentPlayer == WHITE_PLAYER)
                 //   break;

                if(sendCMD == COMMAND_SAVE || sendCMD == COMMAND_IMPORT || sendCMD == COMMAND_LOAD || sendCMD == COMMAND_DISPLAY)
                    printf("[Server] Enter a new message: ");
                else
                    printf("[Server] Enter a valid message: ");

                memset(buffer, 0, BUFFER_SIZE);
                fgets(buffer, BUFFER_SIZE, stdin);
                buffer[strlen(buffer)-1] = '\0';
                sendCMD = send_command(&game, buffer, connfd, false);
                
            }
        }

        //printf("loop skipped");

        if (sendCMD == COMMAND_FORFEIT)
        {
            printf("[Server] Server forfeiting...\n");
            break;
        }
    }

    printf("[Server] Shutting down.\n");
    closesocket(listenfd);
    closesocket(connfd);
    WSACleanup();
    return 0;
}
