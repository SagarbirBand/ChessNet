#include "ChessNet.h"

int main() 
{
    int listenfd, connfd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set options to reuse the IP address and IP port if either is already in use
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) 
    {
        perror("setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))");
        return -1;
    }
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))) 
    {
        perror("setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))");
        return -1;
    }

    // Bind socket to port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(listenfd, (struct sockaddr *)&address, sizeof(address)) < 0) 
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(listenfd, 1) < 0) 
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    INFO("Server listening on port %d", PORT);
    // Accept incoming connection
    if ((connfd = accept(listenfd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) 
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

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
        int nbytes = read(connfd, buffer, BUFFER_SIZE);

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

        
        printf("[Server] Enter message: ");
        memset(buffer, 0, BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strlen(buffer)-1] = '\0';
        sendCMD = send_command(&game, buffer, connfd, false);

        while(!(sendCMD == COMMAND_MOVE || sendCMD == COMMAND_FORFEIT))
        {
            if(sendCMD == COMMAND_SAVE || sendCMD == COMMAND_IMPORT || sendCMD == COMMAND_LOAD || sendCMD == COMMAND_DISPLAY)
                printf("[Server] Enter a new message: ");
            else
                printf("[Server] Enter a valid message: ");

            memset(buffer, 0, BUFFER_SIZE);
            fgets(buffer, BUFFER_SIZE, stdin);
            buffer[strlen(buffer)-1] = '\0';
            sendCMD = send_command(&game, buffer, connfd, false);
            
        }

        if (sendCMD == COMMAND_FORFEIT)
        {
            printf("[Server] Server forfeiting...\n");
            break;
        }
    }

    printf("[Server] Shutting down.\n");
    close(listenfd);
    close(connfd);
    return 0;
}
