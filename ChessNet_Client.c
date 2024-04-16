#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#include "ChessNet.h"

#pragma comment(lib, "ws2_32.lib") // Link with ws2_32.lib

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsa;
    ChessGame game;
    SOCKET connfd = 0;
    struct sockaddr_in serv_addr;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if ((connfd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(connfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }

       initialize_game(&game);
    display_chessboard(&game);

    char buffer[BUFFER_SIZE] = {0};
    int sendCMD;
    int recCMD;

    while (1)
    {
        printf("[Client] Enter message: ");
        memset(buffer, 0, BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strlen(buffer)-1] = '\0';

        sendCMD = send_command(&game, buffer, connfd, true);
        while(!(sendCMD == COMMAND_MOVE || sendCMD == COMMAND_FORFEIT))
        {

            if((sendCMD == COMMAND_IMPORT || sendCMD == COMMAND_LOAD) && game.currentPlayer == BLACK_PLAYER)
                break;

            if(sendCMD == COMMAND_SAVE || sendCMD == COMMAND_IMPORT || sendCMD == COMMAND_LOAD || sendCMD == COMMAND_DISPLAY)
                printf("[Client] Saved. Enter a message: ");
            else
                printf("[Client] Enter a valid message: ");\

            memset(buffer, 0, BUFFER_SIZE);
            fgets(buffer, BUFFER_SIZE, stdin);
            buffer[strlen(buffer)-1] = '\0';
            sendCMD = send_command(&game, buffer, connfd, true);
        }

        if (sendCMD == COMMAND_FORFEIT)
        {
            printf("[Client] Client forfeiting...\n");
            break;
        }

        memset(buffer, 0, BUFFER_SIZE);
        int nbytes = recv(connfd, buffer, BUFFER_SIZE, 0);

        if (nbytes <= 0) 
        {
            perror("[Server] read() failed.");
            break;
        }

        printf("[Server] Received from server: %s\n", buffer);
        recCMD = receive_command(&game, buffer, connfd, true);
        if (recCMD == COMMAND_FORFEIT) 
        {
            printf("[Server] Server Forfeiting...\n");
            break;
        }

    }

    printf("[Server] Shutting down.\n");

    // Please ensure that the following lines of code execute just before your program terminates.
    // If necessary, copy and paste it to other parts of your code where you terminate your program.
    display_chessboard(&game);
    FILE *temp = fopen("./fen.txt", "w");
    char fen[200];
    chessboard_to_fen(fen, &game);
    fprintf(temp, "%s", fen);
    fclose(temp);
    closesocket(connfd);
    WSACleanup();
    return 0;
}
